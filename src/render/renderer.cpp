#include "hop.h"
#include "render/renderer.h"
#include "render/gl_window.h"
#include "render/drawing.h"
#include "render/film.h"
#include "render/tile.h"
#include "geometry/world.h"
#include "util/log.h"
#include "util/stop_watch.h"
#include "camera/camera.h"
#include "camera/camera_sample.h"
#include "camera/trackball.h"
#include "math/math.h"
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "integrators/path_integrator.h"
#include "integrators/ambient_occlusion.h"
#include "integrators/debug_integrator.h"
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
    , m_ctrl_pressed(false), m_options(options)
{
    m_integrator = std::make_unique<PathIntegrator>(m_world);
    m_trackball = std::make_unique<TrackBall>(m_camera, this);

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
            m_integrator = std::make_unique<AmbientOcclusionIntegrator>(m_world);
            reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_N)
        {
            m_integrator = std::make_unique<DebugIntegrator>(m_world, DebugIntegrator::NORMALS);
            reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_P)
        {
            m_integrator = std::make_unique<DebugIntegrator>(m_world, DebugIntegrator::POSITION);
            reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_U)
        {
            m_integrator = std::make_unique<DebugIntegrator>(m_world, DebugIntegrator::UVS);
            reset();
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_T)
        {
            m_integrator = std::make_unique<PathIntegrator>(m_world);
            reset();
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

void Renderer::reset()
{
    std::unique_lock<std::mutex> tiles_lock(m_tiles_mutex);

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
                m_tiles_mutex.unlock();

                render_tile(tile, m_options.tile_spp);

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

            draw::Buffer<Vec3f> buf;
            buf.buffer = framebuffer;
            buf.pitch = m_options.frame_size.x;
            buf.width = m_options.frame_size.x;
            buf.height = m_options.frame_size.y;
            std::ostringstream oss;
            oss << num_samples << " spp - " << rays_per_s << " krays/s";

            draw::bar(buf, Vec2u(0, 0), Vec2u(200, 30), Vec3f(0, 0, 0));
            draw::print(buf, oss.str().c_str(), Vec2u(10, 10), Vec3f(1, 1, 1));

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
void Renderer::render_subtile(const Tile& tile, uint32 spp, bool reset)
{
    Vec3r color(0, 0, 0);
    Real rcp_spp = rcp((Real)spp);
    for (uint32 k = 0; k < spp; ++k)
    {
        // Generate the samples according to a tent filter
        Real r1 = 2 * random<Real>();
        Real r2 = 2 * random<Real>();
        Real dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
        Real dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
        dx = (dx + 1) * 0.5;
        dy = (dy + 1) * 0.5;

        CameraSample sample;
        sample.lens_point = Vec2r(random<Real>(), random<Real>());
        sample.film_point = Vec2r((Real)tile.x + dx * (Real)tile.w,
                                  (Real)tile.y + dy * (Real)tile.h);

        Ray ray;
        Real ray_w = m_camera->generate_ray(sample, &ray);
        color = color + m_integrator->get_radiance(ray) * ray_w * rcp_spp;
    }

    if (reset)
        for (uint32 j = 0; j < tile.h; ++j)
            for (uint32 i = 0; i < tile.w; ++i)
                m_film->reset_pixel(tile.x + i, tile.y + j);

    for (uint32 j = 0; j < tile.h; ++j)
        for (uint32 i = 0; i < tile.w; ++i)
            m_film->add_sample(tile.x + i, tile.y + j, color);
}

// Recursively renders the four subtiles of a tile
void Renderer::render_subtile_divide(const Tile& tile, const Tile& subtile, uint32 res, uint32 spp, bool reset)
{
    if (subtile.w <= res && subtile.h <= res)
    {
        Tile tile_to_render = { tile.x + subtile.x, tile.y + subtile.y, subtile.w, subtile.h, 0 };
        render_subtile(tile_to_render, spp, reset);
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
        if (cbl.w && cbl.h) render_subtile_divide(tile, cbl, res, spp, reset);
        if (cbr.w && cbr.h) render_subtile_divide(tile, cbr, res, spp, reset);
        if (ctl.w && ctl.h) render_subtile_divide(tile, ctl, res, spp, reset);
        if (ctr.w && ctr.h) render_subtile_divide(tile, ctr, res, spp, reset);
    }
}

// Render a tile with the given number of samples per pixel
// The method returns the actual number of spp used, which may be different in preview mode
void Renderer::render_tile(const Tile& tile, uint32 spp)
{
    // Give a preview of the render by rendering using
    // a resolution a one sample per tile and increasing the resolution by 4 (2 for x and y)
    // at each call to render_tile.
    if (m_options.preview && tile.n <= (uint32)max(log2(tile.w), log2(tile.h)))
    {
        uint32 res = max(1u, max(tile.w, tile.h) / (1 << tile.n));
        Tile subtile = { 0, 0, tile.w, tile.h, 0 };
        // we ask for a framebuffer reset after each iteration
        render_subtile_divide(tile, subtile, res, m_options.tile_preview_spp, true);
    }
    // Once the final resolution is reached, we can render normally
    else
    {
        // Render each pixel
        for (uint32 j = 0; j < tile.h; ++j)
        {
            for (uint32 i = 0; i < tile.w; ++i)
            {
                Tile tile_to_render = { tile.x + i, tile.y + j, 1, 1, 0 };
                render_subtile(tile_to_render, spp, false);
            }
        }
    }
}

// Copy the accumulation buffer to the screen an apply the neccessary postprocessing
void Renderer::postprocess_buffer_and_display(Vec3f* framebuffer, uint32 size_x, uint32 size_y)
{
    const Film::Pixel* pixels = m_film->get_pixels();
    const float inv_gamma = 1.0f / 2.2f;

    for (uint32 i = 0; i < size_y; ++i)
    {
        for (uint32 j = 0; j < size_x; ++j)
        {
            const Vec3r col = pixels[i * size_x + j].color;

            framebuffer[i * size_x + j] =
                Vec3f(pow(float(col.x), inv_gamma),
                      pow(float(col.y), inv_gamma),
                      pow(float(col.z), inv_gamma));
        }
    }
}

} // namespace hop
