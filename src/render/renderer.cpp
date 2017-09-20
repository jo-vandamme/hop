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
#include "integrators/ambient_occlusion.h"
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
    m_integrator = std::make_unique<AmbientOcclusionIntegrator>(m_world);

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
    std::unique_lock<std::mutex> fb_lock(m_framebuffer_mutex);

    for (uint32 i = 0; i < m_options.frame_size.y; ++i)
        for (uint32 j = 0; j < m_options.frame_size.x; ++j)
            m_accum_buffer[i * m_options.frame_size.x + j] = Vec3r(0, 0, 0);

    std::unique_lock<std::mutex> tiles_lock(m_tiles_mutex);

    m_total_spp = 0;
    m_next_free_tile = 0;
    m_num_tiles_drawn = 0;

    for (uint32 i = 0; i < m_tiles_infos.size(); ++i)
    {
        m_tiles_infos[i].num_iters = 0;
        m_tiles_infos[i].num_samples = 0;
    }
}

void Renderer::init_tiles_spiral()
{
    std::unique_lock<std::mutex> lock(m_tiles_mutex);

    m_tiles.clear();
    m_tiles_infos.clear();
    const int fw = m_options.frame_size.x;
    const int fh = m_options.frame_size.y;
    const int tw = m_options.tile_size.x;
    const int th = m_options.tile_size.y;

    TileInfo info;
    info.num_samples = 0;
    info.num_iters = 0;

    // This function gets a tile coordinate and calculates the
    // corresponding tile, if the tile is non-empty, it pushed into the list
    auto make_tile = [&](int i, int j)
    {
        Tile tile;
        tile.x = (int)max(0.0f, min(((float)i - 1.0f) * tw + (float)fw / 2.0f, (float)fw));
        tile.y = (int)max(0.0f, min(((float)j - 1.0f) * th + (float)fh / 2.0f, (float)fh));
        tile.w = min(fw - (float)tile.x, (float)tw);
        tile.h = min(fh - (float)tile.y, (float)th);

        if (tile.w != 0 && tile.h != 0)
        {
            m_tiles.push_back(tile);
            m_tiles_infos.push_back(info);
        }
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
    m_tiles_infos.clear();

    TileInfo info;
    info.num_samples = 0;
    info.num_iters = 0;

    for (uint32 y = 0; y < m_options.frame_size.y; y += m_options.tile_size.y)
    {
        Tile tile;
        tile.y = y;
        tile.h = min(m_options.frame_size.y - y, m_options.tile_size.y);

        for (uint32 x = 0; x < m_options.frame_size.x; x += m_options.tile_size.x)
        {
            tile.x = x;
            tile.w = min(m_options.frame_size.x - x, m_options.tile_size.x);
            m_tiles.push_back(tile);
            m_tiles_infos.push_back(info);
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
            std::unique_ptr<Vec3r[]> buffer = std::make_unique<Vec3r[]>(m_options.tile_size.x * m_options.tile_size.y);

            while (!rendering_done)
            {
                if (!interactive && m_next_free_tile >= m_tiles.size())
                    break;

                // Get a tile and render it
                m_tiles_mutex.lock();
                uint32 tile_idx = m_next_free_tile++ % m_tiles.size();
                Tile tile = m_tiles[tile_idx];
                TileInfo info = m_tiles_infos[tile_idx];
                m_tiles_mutex.unlock();

                bool reset = false;
                uint32 num_samples = render_tile(&buffer[0], tile, info, m_options.tile_spp, reset);

                // Increase the sample count for this tile
                m_tiles_mutex.lock();
                if (reset)
                    m_tiles_infos[tile_idx].num_samples = 0;
                m_tiles_infos[tile_idx].num_samples += num_samples;
                ++m_tiles_infos[tile_idx].num_iters;
                uint32 total_samples = m_tiles_infos[tile_idx].num_samples;
                m_tiles_mutex.unlock();

                if (tile_idx == 0)
                    m_total_spp = total_samples;

                // Add the tile buffer to the accumulator and scale the value accordingly
                m_framebuffer_mutex.lock();
                for (uint32 i = 0; i < tile.h; ++i)
                {
                    for (uint32 j = 0; j < tile.w; ++j)
                    {
                        uint32 idx = (tile.y + i) * m_options.frame_size.x + tile.x + j;
                        m_accum_buffer[idx] = reset ?
                            buffer[i * tile.w + j] * rcp((Real)num_samples) :
                            (m_accum_buffer[idx] * (Real)(total_samples - num_samples) +
                             buffer[i * tile.w + j]) * rcp((Real)total_samples);
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

            Log("render") << INFO << m_total_spp << " spp - "
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

// Renders a subtile, spp rays are shot from the tile to determine the tile's uniform color
void Renderer::render_subtile(Vec3r* buffer, const Tile& tile, const Tile& subtile, uint32 spp)
{
    Vec3r color(0, 0, 0);
    for (uint32 k = 0; k < spp; ++k)
    {
        // Generate the samples according to a tent filter
        Real r1 = 2 * random<Real>();
        Real r2 = 2 * random<Real>();
        Real dx = r1 < 1 ? sqrt(r1) - 1: 1 - sqrt(2 - r1);
        Real dy = r2 < 1 ? sqrt(r2) - 1: 1 - sqrt(2 - r2);
        dx = (dx + 1) * 0.5;
        dy = (dy + 1) * 0.5;

        CameraSample sample;
        sample.lens_point = Vec2r(random<Real>(), random<Real>());
        sample.film_point = Vec2r((Real)(tile.x + subtile.x) + dx * (Real)subtile.w,
                                  (Real)(tile.y + subtile.y) + dy * (Real)subtile.h);

        Ray ray;
        Real ray_w = m_camera->generate_ray(sample, &ray);
        color = color + m_integrator->get_radiance(ray) * ray_w;
    }

    for (uint32 j = 0; j < subtile.h; ++j)
    {
        for (uint32 i = 0; i < subtile.w; ++i)
        {
            uint32 idx = (subtile.y + j) * tile.w + subtile.x + i;
            buffer[idx] = color;
        }
    }
}

// Recursively renders the four subtiles of a tile
void Renderer::render_subtile_corners(Vec3r* buffer, const Tile& tile, const Tile& subtile, uint32 res, uint32 spp)
{
    if (subtile.w <= res)
    {
        render_subtile(buffer, tile, subtile, spp);
    }
    else
    {
        uint32 size = subtile.w / 2;
        Tile corner0 = { subtile.x,        subtile.y,        size, size };
        Tile corner1 = { subtile.x + size, subtile.y,        size, size };
        Tile corner2 = { subtile.x,        subtile.y + size, size, size };
        Tile corner3 = { subtile.x + size, subtile.y + size, size, size };
        render_subtile_corners(buffer, tile, corner0, res, spp);
        render_subtile_corners(buffer, tile, corner1, res, spp);
        render_subtile_corners(buffer, tile, corner2, res, spp);
        render_subtile_corners(buffer, tile, corner3, res, spp);
    }
}

// Render a tile with the given number of samples per pixel
// The method returns the actual number of spp used, which may be different in preview mode
uint32 Renderer::render_tile(Vec3r* buffer, const Tile& tile, const TileInfo& info, uint32 spp, bool& reset)
{
    bool preview = m_options.preview && (tile.w == tile.h) && is_power_of_2(tile.w);

    // If the tile is square and the size is a power of 2
    // then we can offer a preview of the render by rendering using
    // a resolution a one sample per tile and increasing the resolution by 4 (2 for x and y)
    // at each call to render_tile.
    if (preview && info.num_iters <= (uint32)log2(tile.w))
    {
        // we ask for a framebuffer reset after each iteration
        reset = true;
        spp = m_options.tile_preview_spp;

        uint32 res = tile.w / (1 << info.num_iters);
        Tile subtile = { 0, 0, tile.w, tile.h };
        render_subtile_corners(buffer, tile, subtile, res, spp);
    }
    // Once the final resolution is reached, we can render normally
    else
    {
        // Render each pixel
        for (uint32 j = 0; j < tile.h; ++j)
        {
            for (uint32 i = 0; i < tile.w; ++i)
            {
                Tile subtile = { i, j, 1, 1 };
                render_subtile(buffer, tile, subtile, spp);
            }
        }
    }
    return spp;
}

// Copy the accumulation buffer to the screen an apply the neccessary postprocessing
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
