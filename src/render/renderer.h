#pragma once

#include "types.h"
#include "options.h"
#include "camera/camera.h"
#include "geometry/world.h"
#include "geometry/ray.h"
#include "render/gl_window.h"
#include "math/vec3.h"
#include "lua/environment.h"
#include "util/log.h"

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
    void set_camera(std::shared_ptr<Camera> camera);
    void reset();

    void set_lua_environment(lua::Environment* env) { m_lua = env; }

private:
    struct Tile
    {
        uint32 x, y, w, h;
    };

    struct TileInfo
    {
        uint32 num_iters, num_samples;
    };

    void init_tiles_linear();
    void init_tiles_spiral();
    uint32 render_tile(Vec3r* buffer, const Tile& tile, const TileInfo& info, uint32 spp, bool& reset);
    void render_subtile(Vec3r* buffer, const Tile& tile, const Tile& subtile, uint32 spp);
    void render_subtile_corners(Vec3r* buffer, const Tile& tile, const Tile& subtile, uint32 res, uint32 spp);
    void postprocess_buffer_and_display(Vec3f* framebuffer, Vec3r* image, uint32 size_x, uint32 size_y);
    Vec3r get_radiance(const Ray& ray);

private:
    std::unique_ptr<GLWindow> m_window;
    std::shared_ptr<World> m_world;
    std::shared_ptr<Camera> m_camera;
    std::mutex m_framebuffer_mutex;
    std::mutex m_tiles_mutex;
    std::unique_ptr<Vec3r[]> m_accum_buffer;
    std::vector<Tile> m_tiles;
    std::vector<TileInfo> m_tiles_infos;
    std::atomic<uint32> m_num_tiles_drawn;
    uint32 m_next_free_tile;
    uint32 m_total_spp;
    Options m_options;
    lua::Environment* m_lua;
};

} // namespace hop
