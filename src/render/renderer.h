#pragma once

#include "types.h"
#include "options.h"
#include "camera/camera.h"
#include "camera/trackball.h"
#include "geometry/world.h"
#include "geometry/ray.h"
#include "render/gl_window.h"
#include "render/film.h"
#include "render/tile.h"
#include "render/tonemap.h"
#include "math/vec3.h"
#include "lua/environment.h"
#include "util/log.h"
#include "integrators/integrator.h"

#include <memory>
#include <mutex>
#include <functional>

namespace hop {

class Renderer
{
public:
    Renderer(std::shared_ptr<World> world, std::shared_ptr<Camera> camera, const Options& options);
    ~Renderer() { Log("renderer") << DEBUG << "renderer deleted"; }

    int render(bool interactive = true);
    void reset();

    void set_lua_environment(lua::Environment* env) { m_lua = env; }

private:
    void render_tile(const Tile& tile, uint32 spp, std::shared_ptr<Integrator> integrator);
    void render_subtile(const Tile& tile, uint32 spp, bool reset, std::shared_ptr<Integrator> integrator);
    void render_subtile_divide(const Tile& tile, const Tile& subtile, uint32 res, uint32 spp, bool reset, std::shared_ptr<Integrator> integrator);

    void postprocess_buffer_and_display(Vec3f* framebuffer, uint32 size_x, uint32 size_y);

    enum IntegratorMode
    {
        PATH,
        OCCLUSION,
        POSITION,
        NORMALS,
        UVS
    };

    enum DisplayMode
    {
        COLOR,
        VARIANCE,
        SAMPLES
    };

private:
    std::unique_ptr<GLWindow> m_window;
    std::shared_ptr<World> m_world;
    std::shared_ptr<Camera> m_camera;
    std::unique_ptr<TrackBall> m_trackball;
    std::mutex m_tiles_mutex;
    std::unique_ptr<Film> m_film;
    std::vector<Tile> m_tiles;
    std::shared_ptr<Integrator> m_integrator;
    uint32 m_next_free_tile;
    bool m_ctrl_pressed;
    Options m_options;
    lua::Environment* m_lua;
    IntegratorMode m_integrator_mode;
    DisplayMode m_display_mode;
    uint32 m_num_adaptive_samples;
    uint32 m_num_firefly_samples;
    Real m_adaptive_exponent;
    Real m_adaptive_threshold;
    Real m_firefly_threshold;
    ToneMapType m_tonemap;
};

} // namespace hop
