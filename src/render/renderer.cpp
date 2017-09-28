#include "hop.h"
#include "render/renderer.h"
#include "render/gl_window.h"
#include "render/drawing.h"
#include "render/film.h"
#include "render/tile.h"
#include "render/tonemap.h"
#include "geometry/world.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"
#include "util/log.h"
#include "util/stop_watch.h"
#include "camera/camera.h"
#include "camera/projective_camera.h"
#include "camera/camera_sample.h"
#include "camera/trackball.h"
#include "math/math.h"
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "integrator/pathtracing.h"
#include "integrator/ao.h"
#include "integrator/debug.h"
#include "spectrum/spectrum.h"
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
    , m_world(world), m_camera(camera), m_next_free_tile(0)
    , m_ctrl_pressed(false), m_options(options), m_integrator_mode(PATH), m_display_mode(COLOR)
    , m_num_adaptive_samples(options.adaptive_spp), m_num_firefly_samples(options.firefly_spp)
    , m_adaptive_exponent(options.adaptive_exponent)
    , m_adaptive_threshold(options.adaptive_threshold), m_firefly_threshold(options.firefly_threshold)
    , m_tonemap(options.tonemap)
{
    m_trackball = std::make_unique<TrackBall>(m_camera, this);

    reset();

    m_window->set_key_handler([&](int key, int /*scancode*/, int action, int /*mods*/)
    {
        if (action == GLFW_PRESS && key == GLFW_KEY_LEFT_CONTROL)
        {
            m_ctrl_pressed = true;
        }
        else if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT_CONTROL)
        {
            m_ctrl_pressed = false;
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_HOME)
        {
            m_trackball->reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_O)
        {
            m_integrator_mode = OCCLUSION;
            reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_N)
        {
            m_integrator_mode = NORMALS;
            reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_P)
        {
            m_integrator_mode = POSITION;
            reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_U)
        {
            m_integrator_mode = UVS;
            reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_T)
        {
            m_integrator_mode = PATH;
            reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_I)
        {
            m_display_mode = SAMPLES;
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_V)
        {
            m_display_mode = VARIANCE;
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_C)
        {
            m_display_mode = COLOR;
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_G)
        {
            m_tonemap = ToneMapType::GAMMA;
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_R)
        {
            m_tonemap = ToneMapType::REINHARD;
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_F)
        {
            m_tonemap = ToneMapType::FILMIC;
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_L)
        {
            m_tonemap = ToneMapType::LINEAR;
        }

        if (m_lua)
            m_lua->call("key_handler", "ii", key, action);

        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
            m_window->set_should_close(true);
    });

    m_window->set_cursor_pos_handler([&](double x, double y)
    {
        if (m_ctrl_pressed)
            m_trackball->on_motion(x, y);
        if (m_lua)
            m_lua->call("cursor_pos_handler", "dd", x, y);
    });

    m_window->set_mouse_button_handler([&](int button, int action, int mods)
    {
        if (m_ctrl_pressed && action == GLFW_PRESS)
            m_trackball->on_button_down(button);
        else if (m_ctrl_pressed && action == GLFW_RELEASE)
            m_trackball->on_button_up(button);
        if (m_lua)
            m_lua->call("mouse_button_handler", "iii", button, action, mods);
    });

    m_window->init();
}

Real Renderer::set_focus_point(const Vec2r& point)
{
    CameraSample sample;
    sample.lens_point = Vec2r(0, 0);
    sample.film_point = Vec2r(point.x, m_options.frame_size.y - point.y);
    Ray ray;
    m_camera->generate_ray(sample, &ray);

    Real dist = (Real)pos_inf;
    HitInfo hit;
    if (m_world->intersect(ray, &hit))
    {
        SurfaceInteraction isect;
        m_world->get_surface_interaction(hit, &isect);
        Vec3r pos = isect.position;
        dist = length(pos - m_camera->get_eye());

        auto cam = std::dynamic_pointer_cast<ProjectiveCamera>(m_camera);
        if (cam)
        {
            cam->set_focal_distance(dist);
            reset();
        }
    }
    return dist;
}

void Renderer::reset()
{
    std::unique_lock<std::mutex> tiles_lock(m_tiles_mutex);

    switch (m_integrator_mode)
    {
        case OCCLUSION:
            m_integrator = std::make_shared<AmbientOcclusionIntegrator>(m_world);
            break;
        case POSITION:
            m_integrator = std::make_shared<DebugIntegrator>(m_world, DebugIntegrator::POSITION);
            break;
        case NORMALS:
            m_integrator = std::make_shared<DebugIntegrator>(m_world, DebugIntegrator::NORMALS);
            break;
        case UVS:
            m_integrator = std::make_shared<DebugIntegrator>(m_world, DebugIntegrator::UVS);
            break;
        case PATH:
        default:
            m_integrator = std::make_shared<PathIntegrator>(m_world);
            break;
    }

    m_next_free_tile = 0;
    for (uint32 i = 0; i < m_tiles.size(); ++i)
        m_tiles[i].n = 0;
}

int Renderer::render(bool interactive)
{
#ifdef TILES_SPIRAL
    m_tiles = make_tiles_spiral(m_options.frame_size.x, m_options.frame_size.y,
                                m_options.tile_size.x, m_options.tile_size.y);
#else
    m_tiles = make_tiles_linear(m_options.frame_size.x, m_options.frame_size.y,
                                m_options.tile_size.x, m_options.tile_size.y);
#endif

    std::atomic<bool> rendering_done(false);
    std::atomic<bool> tile_done(false);
    m_next_free_tile = 0;

    m_film = std::make_unique<Film>(m_options.frame_size.x, m_options.frame_size.y);

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

                // Get a tile and render it
                m_tiles_mutex.lock();
                uint32 tile_idx = m_next_free_tile++ % m_tiles.size();
                Tile tile = m_tiles[tile_idx];
                std::shared_ptr<Integrator> integrator = m_integrator;
                m_tiles_mutex.unlock();

                render_tile(tile, m_options.spp, integrator);

                // Increase the sample count for this tile
                m_tiles_mutex.lock();
                ++m_tiles[tile_idx].n;
                m_tiles_mutex.unlock();

                tile_done = true;
            }
        }));
    }

    m_window->show();

    StopWatch timer;
    timer.start();

    uint32 rays_per_s = 0;
    uint32 num_samples = 0;

    StopWatch loop_timer;
    loop_timer.start();

    // Poll the window events and update the framebuffer
    while (!m_window->should_close())
    {
        m_window->poll_events();
        m_trackball->update(loop_timer.get_elapsed_time_ms());

        /*TileInfo tile_info = m_tiles_infos[0];
        if (tile_info.num_iters != last_iter)
        {
            uint32 num_rays = m_options.frame_size.x * m_options.frame_size.y * (tile_info.num_samples - num_samples);
            rays_per_s = num_rays / timer.get_elapsed_time_ms();
            num_samples = tile_info.num_samples;
            last_iter = tile_info.num_iters;
            timer.start();
        }
        */

        if (tile_done)
        {
            tile_done = false;
            // copy tile to framebuffer and swap the window's framebuffer
            Vec3f* framebuffer = (Vec3f*)m_window->map_framebuffer();
            postprocess_buffer_and_display(framebuffer, m_options.frame_size.x, m_options.frame_size.y);

            /*
            draw::Buffer<Vec3f> buf;
            buf.buffer = framebuffer;
            buf.pitch = m_options.frame_size.x;
            buf.width = m_options.frame_size.x;
            buf.height = m_options.frame_size.y;
            std::ostringstream oss;
            oss << num_samples << " spp - " << rays_per_s << " krays/s";

            draw::bar(buf, Vec2u(0, 0), Vec2u(200, 30), Vec3f(0, 0, 0));
            draw::print(buf, oss.str().c_str(), Vec2u(10, 10), Vec3f(1, 1, 1));
            */

            m_window->unmap_framebuffer();

            m_window->swap_buffers();
        }
    }

    rendering_done = true;
    for (auto& rt : render_threads)
        rt.join();

    return 0;
}

// Renders a tile, spp rays are shot from the tile to determine the tile's uniform color
void Renderer::render_subtile(const Tile& tile, uint32 spp, bool reset, std::shared_ptr<Integrator> integrator)
{
    if (reset)
        for (uint32 j = 0; j < tile.h; ++j)
            for (uint32 i = 0; i < tile.w; ++i)
                m_film->reset_pixel(tile.x + i, tile.y + j);

    auto render = [&](uint32 spp)
    {
        for (uint32 k = 0; k < spp; ++k)
        {
            Real dx = random<Real>();
            Real dy = random<Real>();
            CameraSample sample;
            sample.lens_point = Vec2r(random<Real>() * 1.0 - 0.5, random<Real>() * 1.0 - 0.5);
            sample.film_point = Vec2r((Real)tile.x + 0.5 + dx * (Real)tile.w,
                                  (Real)tile.y + 0.5 + dy * (Real)tile.h);
            Ray ray;
            Real ray_w = m_camera->generate_ray(sample, &ray);
            Spectrum color = integrator->Li(ray) * ray_w;

            for (uint32 j = 0; j < tile.h; ++j)
                for (uint32 i = 0; i < tile.w; ++i)
                    m_film->add_sample(tile.x + i, tile.y + j, color);
        }
    };

    // Render with spp samples per pixel
    render(spp);

    // Render with an adaptive number of samples per pixel proportional to the standard deviation
    if (m_num_adaptive_samples > 0 && tile.n != 0 && tile.w == 1 && tile.h == 1)
    {
        Real stddev = m_film->get_standard_deviation(tile.x, tile.y);
        Real v = pow(clamp(stddev / m_adaptive_threshold, 0.0, 1.0), m_adaptive_exponent);
        uint32 num_adaptive_samples = (uint32)(v * (Real)m_num_adaptive_samples);
        if (num_adaptive_samples > 0)
            render(num_adaptive_samples);
    }

    // Render num_firefly_samples if the standard deviation is > than the threshold
    if (m_num_firefly_samples > 0 && tile.n != 0 && tile.w == 1 && tile.h == 1)
    {
        Real stddev = m_film->get_standard_deviation(tile.x, tile.y);
        if (stddev > m_firefly_threshold)
            render(m_num_firefly_samples);
    }
}

// Recursively renders the four subtiles of a tile
void Renderer::render_subtile_divide(const Tile& tile, const Tile& subtile, uint32 res, uint32 spp, bool reset, std::shared_ptr<Integrator> integrator)
{
    if (subtile.w <= res && subtile.h <= res)
    {
        Tile tile_to_render = { tile.x + subtile.x, tile.y + subtile.y, subtile.w, subtile.h, 0 };
        render_subtile(tile_to_render, spp, reset, integrator);
    }
    else
    {
        uint32 wl = subtile.w / 2;
        uint32 wr = subtile.w - wl;
        uint32 hb = subtile.h / 2;
        uint32 ht = subtile.h - hb;
        Tile cbl = { subtile.x,      subtile.y,      wl, hb, 0 };
        Tile cbr = { subtile.x + wl, subtile.y,      wr, hb, 0 };
        Tile ctl = { subtile.x,      subtile.y + hb, wl, ht, 0 };
        Tile ctr = { subtile.x + wl, subtile.y + hb, wr, ht, 0 };
        if (cbl.w && cbl.h) render_subtile_divide(tile, cbl, res, spp, reset, integrator);
        if (cbr.w && cbr.h) render_subtile_divide(tile, cbr, res, spp, reset, integrator);
        if (ctl.w && ctl.h) render_subtile_divide(tile, ctl, res, spp, reset, integrator);
        if (ctr.w && ctr.h) render_subtile_divide(tile, ctr, res, spp, reset, integrator);
    }
}

// Render a tile with the given number of samples per pixel
// The method returns the actual number of spp used, which may be different in preview mode
void Renderer::render_tile(const Tile& tile, uint32 spp, std::shared_ptr<Integrator> integrator)
{
    // Give a preview of the render by rendering using
    // a resolution a one sample per tile and increasing the resolution by 4 (2 for x and y)
    // at each call to render_tile.
    if (m_options.preview && tile.n <= (uint32)max(log2(tile.w), log2(tile.h)))
    {
        uint32 res = max(1u, max(tile.w, tile.h) / (1 << tile.n));
        Tile subtile = { 0, 0, tile.w, tile.h, 0 };
        // we ask for a framebuffer reset after each iteration
        render_subtile_divide(tile, subtile, res, m_options.preview_spp, true, integrator);
    }
    // Once the final resolution is reached, we can render normally
    else
    {
        // Render each pixel
        for (uint32 j = 0; j < tile.h; ++j)
        {
            for (uint32 i = 0; i < tile.w; ++i)
            {
                Tile tile_to_render = { tile.x + i, tile.y + j, 1, 1, tile.n };
                render_subtile(tile_to_render, spp, false, integrator);
            }
        }
    }
}

// Copy the accumulation buffer to the screen an apply the neccessary postprocessing
void Renderer::postprocess_buffer_and_display(Vec3f* framebuffer, uint32 size_x, uint32 size_y)
{
    const Film::Pixel* pixels = m_film->get_pixels();

    if (m_display_mode == COLOR)
    {
        for (uint32 i = 0; i < size_y; ++i)
        {
            for (uint32 j = 0; j < size_x; ++j)
            {
                const Spectrum col = pixels[i * size_x + j].color;
                framebuffer[i * size_x + j] = tonemap(m_tonemap, col);
            }
        }
    }
    else if (m_display_mode == VARIANCE)
    {
        for (uint32 i = 0; i < size_y; ++i)
        {
            for (uint32 j = 0; j < size_x; ++j)
            {
                const Real dev = sqrt(pixels[i * size_x + j].variance);
                framebuffer[i * size_x + j] = Vec3f(dev, dev, dev);
            }
        }
    }
    else if (m_display_mode == SAMPLES)
    {
        for (uint32 i = 0; i < size_y; ++i)
        {
            for (uint32 j = 0; j < size_x; ++j)
            {
                float n = float(pixels[i * size_x + j].num_samples) / 1000.0f;
                framebuffer[i * size_x + j] = Vec3f(n, n, n);
            }
        }
    }
}

} // namespace hop
