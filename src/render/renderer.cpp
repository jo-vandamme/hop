#include "hop.h"
#include "render/renderer.h"
#include "render/gl_window.h"
#include "geometry/world.h"
#include "util/log.h"
#include "util/stop_watch.h"
#include "camera/camera.h"
#include "camera/camera_sample.h"
#include "math/math.h"
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "options.h"
#include "types.h"

#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstring>

namespace hop {

Renderer::Renderer(std::shared_ptr<World> world, std::shared_ptr<Camera> camera, const Options& options)
    : m_window(std::make_unique<GLWindow>(options.frame_size.x, options.frame_size.y, "Hop renderer"))
    , m_world(world), m_camera(camera), m_num_tiles_drawn(0), m_next_free_tile(0)
    , m_total_spp(0), m_options(options)
{
    m_window->set_key_handler([&](int key, int /*scancode*/, int action, int /*mods*/)
    {
        if (m_lua)
            m_lua->call("key_handler", "ii", key, action);

        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
            m_window->set_should_close(true);
    });

    m_window->set_cursor_pos_handler([&](double x, double y)
    {
        if (m_lua)
            m_lua->call("cursor_pos_handler", "dd", x, y);
    });

    m_window->set_mouse_button_handler([&](int button, int action, int mods)
    {
        if (m_lua)
            m_lua->call("mouse_button_handler", "iii", button, action, mods);
    });

    m_window->init();
}

void Renderer::set_camera(std::shared_ptr<Camera> camera)
{
    m_camera = camera;
    reset();
}

void Renderer::reset()
{
    m_total_spp = 0;
    std::unique_lock<std::mutex> fb_lock(m_framebuffer_mutex);

    for (uint32 i = 0; i < m_options.frame_size.y; ++i)
        for (uint32 j = 0; j < m_options.frame_size.x; ++j)
            m_accum_buffer[i * m_options.frame_size.x + j] = Vec3r(0, 0, 0);

#ifdef TILES_SPIRAL
    init_tiles_spiral();
#else
    init_tiles_linear();
#endif

    std::unique_lock<std::mutex> tiles_lock(m_tiles_mutex);
    m_next_free_tile = 0;
    m_num_tiles_drawn = 0;
}

void Renderer::init_tiles_spiral()
{
    std::unique_lock<std::mutex> lock(m_tiles_mutex);

    m_tiles.clear();
    const int fw = m_options.frame_size.x;
    const int fh = m_options.frame_size.y;
    const int tw = m_options.tile_size.x;
    const int th = m_options.tile_size.y;

    // This function gets a tile coordinate and calculates the
    // corresponding tile, if the tile is non-empty, it pushed into the list
    auto make_tile = [&](int i, int j)
    {
        TileInfo tile;
        tile.n = 0;
        tile.x = (int)max(0.0f, min(((float)i - 1.0f) * tw + (float)fw / 2.0f, (float)fw));
        tile.y = (int)max(0.0f, min(((float)j - 1.0f) * th + (float)fh / 2.0f, (float)fh));
        tile.w = min(fw - tile.x, tw);
        tile.h = min(fh - tile.y, th);

        if (tile.w != 0 && tile.h != 0)
            m_tiles.push_back(tile);
    };

    int X = fw / tw + 2;
    int Y = fh / th + 2;
    int x, y, dx, dy;
    x = y = dx = 0;
    dy = -1;

    int t = max(X, Y);
    int max_iters = t * t;

    for (int i = 0; i < max_iters; ++i)
    {
        make_tile(x, y);

        if ((x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1-y)))
        {
            t = dx;
            dx = -dy;
            dy = t;
        }
        x += dx;
        y += dy;
    }
}

void Renderer::init_tiles_linear()
{
    std::unique_lock<std::mutex> lock(m_tiles_mutex);

    m_tiles.clear();
    for (uint32 y = 0; y < m_options.frame_size.y; y += m_options.tile_size.y)
    {
        TileInfo tile;
        tile.n = 0;
        tile.y = y;
        tile.h = min(m_options.frame_size.y - y, m_options.tile_size.y);

        for (uint32 x = 0; x < m_options.frame_size.x; x += m_options.tile_size.x)
        {
            tile.x = x;
            tile.w = min(m_options.frame_size.x - x, m_options.tile_size.x);
            m_tiles.push_back(tile);
        }
    }
}

int Renderer::render(bool interactive)
{
#ifdef TILES_SPIRAL
    init_tiles_spiral();
#else
    init_tiles_linear();
#endif

    std::atomic<bool> rendering_done(false);
    std::atomic<bool> tile_done(false);
    m_next_free_tile = 0;

    m_accum_buffer = std::make_unique<Vec3r[]>(m_options.frame_size.x * m_options.frame_size.y);
    std::memset(&m_accum_buffer[0], 0, m_options.frame_size.x * m_options.frame_size.y * sizeof(Vec3r));

    // Spawn the render threads
    int num_threads = std::thread::hardware_concurrency() - 1;
    std::vector<std::thread> render_threads;

    for (int i = 0; i < num_threads; ++i)
    {
        render_threads.push_back(std::thread([&]()
        {
            while (!rendering_done)
            {
                if (!interactive && m_next_free_tile >= m_tiles.size())
                    break;

                // get a tile and render it
                m_tiles_mutex.lock();
                uint32 tile_idx = m_next_free_tile++ % m_tiles.size();
                const uint32 spp = m_options.tile_spp;
                m_tiles[tile_idx].n += spp;
                TileInfo tile = m_tiles[tile_idx];
                m_tiles_mutex.unlock();

                std::unique_ptr<Vec3r[]> buffer = std::make_unique<Vec3r[]>(tile.w * tile.h);
                render_tile(&buffer[0], tile.x, tile.y, tile.w, tile.h, spp);

                m_framebuffer_mutex.lock();
                for (int i = 0; i < tile.h; ++i)
                {
                    for (int j = 0; j < tile.w; ++j)
                    {
                        uint32 idx = (tile.y + i) * m_options.frame_size.x + tile.x + j;
                        Real n = (Real)tile.n;
                        m_accum_buffer[idx] = (m_accum_buffer[idx] * (n - spp) + buffer[i * tile.w + j]) * rcp(n);
                    }
                }
                m_framebuffer_mutex.unlock();
                tile_done = true;
                ++m_num_tiles_drawn;
            }
        }));
    }

    m_window->show();

    StopWatch timer;
    timer.start();

    // Poll the window events and update the framebuffer
    while (!m_window->should_close())
    {
        m_window->poll_events();

        float elapsed_time = (float)timer.get_elapsed_time_ms();
        if (elapsed_time > 1000.0f)
        {
            int num_rays = m_num_tiles_drawn * m_options.tile_size.x * m_options.tile_size.y * m_options.tile_spp;
            float rays_per_s = 1000.0f * num_rays / timer.get_elapsed_time_ms();

            Log("render") << INFO << m_tiles[0].n << " spp - "
                                  << std::fixed << std::setprecision(3) << rays_per_s * 0.001f << " krays/s";

            m_num_tiles_drawn = 0;
            timer.start();
        }

        if (tile_done)
        {
            tile_done = false;
            // copy tile to framebuffer and swap the window's framebuffer
            Vec3f* framebuffer = (Vec3f*)m_window->map_framebuffer();
            postprocess_buffer_and_display(
                    framebuffer, &m_accum_buffer[0],
                    m_options.frame_size.x, m_options.frame_size.y);
            m_window->unmap_framebuffer();

            m_window->swap_buffers();
        }
    }

    rendering_done = true;
    for (auto& rt : render_threads)
        rt.join();

    return 0;
}

inline Vec3r Renderer::get_radiance(const Ray& ray)
{
    SurfaceInteraction isect;
    HitInfo hit;
    if (!m_world->intersect(ray, &hit))
        return Vec3r(0, 0, 0);

    m_world->get_surface_interaction(hit, &isect);
    Vec3r n = normalize(isect.normal);

    Vec3r occlusion;
    Real occlusion_amount = 1.0;
    const Real occlusion_step = 1.0 / (Real)NUM_AO_RAYS;
    for (int i = 0; i < NUM_AO_RAYS; ++i)
    {
#if 0
        Real t0 = 2.0 * (Real)pi * random<Real>();
        Real t1 = acos(1.0 - 2.0 * random<Real>());
        Real s0 = sin(t0);
        Real s1 = sin(t1);
        Real c0 = cos(t0);
        Real c1 = cos(t1);
        Real x = s0 * s1;
        Real y = c0 * s1;
        Real z = c1;
        Vec3r random_dir(x, y, z);
#else
        Real x, y, z, d;
        do {
            x = random<Real>() * 2 - 1;
            y = random<Real>() * 2 - 1;
            z = random<Real>() * 2 - 1;
            d = x * x + y * y + z * z;
        } while (d > 1);
        Vec3r random_dir(x / d, y / d, z / d);
        random_dir = normalize(random_dir);
#endif
        if (dot(random_dir, n) < 0)
            random_dir = -random_dir;

        Ray occlusion_ray;
        occlusion_ray.org = isect.position + random_dir * RAY_EPSILON;
        occlusion_ray.dir = random_dir;
        occlusion_ray.tmin = RAY_TMIN;
        occlusion_ray.tmax = RAY_TFAR;
        HitInfo occlusion_hit;
        if (m_world->intersect_any(occlusion_ray, &occlusion_hit))
            occlusion_amount -= occlusion_step;
    }
    occlusion = occlusion_amount * Vec3r(1, 1, 1);

    return occlusion;
}

void Renderer::render_tile(Vec3r* buffer, uint32 tile_x, uint32 tile_y, uint32 tile_w, uint32 tile_h, uint32 spp)
{
    for (uint32 j = 0; j < tile_h; ++j)
    {
        for (uint32 i = 0; i < tile_w; ++i)
        {
            const uint32 idx = j * tile_w + i;
            buffer[idx] = Vec3r(0, 0, 0);
            Vec3r color(0, 0, 0);

            for (uint32 k = 0; k < spp; ++k)
            {
                Ray ray;
                CameraSample sample;
                sample.lens_point = Vec2r(random<Real>(), random<Real>());
                sample.film_point = Vec2r((Real)i + (Real)tile_x + random<Real>(),
                                          (Real)j + (Real)tile_y + random<Real>());
                Real ray_w = m_camera->generate_ray(sample, &ray);

                color = color + get_radiance(ray) * ray_w;

            }
            buffer[idx] = buffer[idx] + color;// * rcp((Real)spp);
        }
    }
}

void Renderer::postprocess_buffer_and_display(
        Vec3f* framebuffer, Vec3r* image, uint32 size_x, uint32 size_y)
{
    float inv_gamma = 1.0f / 2.2f;
    for (uint32 i = 0; i < size_y; ++i)
    {
        for (uint32 j = 0; j < size_x; ++j)
        {
            const Vec3r& col = image[i * size_x + j];
            framebuffer[i * size_x + j] =
                Vec3f(pow(float(col.x), inv_gamma),
                      pow(float(col.y), inv_gamma),
                      pow(float(col.z), inv_gamma));
        }
    }
}

} // namespace hop
