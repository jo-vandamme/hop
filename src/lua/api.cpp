#include "hop.h"
#include "lua/lua.h"
#include "lua/stack.h"
#include "lua/environment.h"
#include "util/log.h"

#include "loaders/obj.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/bbox.h"
#include "math/transform.h"
#include "geometry/world.h"
#include "geometry/shape_manager.h"
#include "camera/perspective_camera.h"
#include "render/renderer.h"
#include "render/tonemap.h"
#include "options.h"

#include <sstream>
#include <memory>
#include <cstdio>
#include <string>

namespace hop { namespace lua {

static Environment* g_environment;
static std::string g_path;

// This is here because GCC was optmizing away the destructor calls
// causing a segfault when during lua exit
template <typename T>
void NOOPTIMIZE destroy(T& obj)
{
    obj.~T();
}

static int safe_getfield_int(lua_State* L, int idx, const char* field, int default_value)
{
    Stack s(L);
    lua_getfield(L, idx, field);
    int value = default_value;
    if (!lua_isnil(L, -1))
        value = s.get_int(-1);
    s.pop(1);
    return value;
}

static bool safe_getfield_bool(lua_State* L, int idx, const char* field, bool default_value)
{
    Stack s(L);
    lua_getfield(L, idx, field);
    bool value = default_value;
    if (!lua_isnil(L, -1))
        value = s.get_bool(-1);
    s.pop(1);
    return value;
}

static double safe_getfield_double(lua_State* L, int idx, const char* field, double default_value)
{
    Stack s(L);
    lua_getfield(L, idx, field);
    double value = default_value;
    if (!lua_isnil(L, -1))
        value = s.get_double(-1);
    s.pop(1);
    return value;
}

static Vec3r safe_getfield_vec3(lua_State* L, int idx, const char* field, Vec3r default_value)
{
    Stack s(L);
    lua_getfield(L, idx, field);
    Vec3r value = default_value;
    if (!lua_isnil(L, -1))
        value = s.get_vec3(-1);
    s.pop(1);
    return value;
}

static const char* safe_getfield_string(lua_State* L, int idx, const char* field, const char* default_value)
{
    Stack s(L);
    lua_getfield(L, idx, field);
    const char* value = default_value;
    if (!lua_isnil(L, -1))
        value = s.get_string(-1);
    s.pop(1);
    return value;
}

static int load_obj(lua_State* L)
{
    Stack s(L);
    const char* file = s.get_string(1);
    ShapeID id = obj::load(file);
    s.push_shape(id);
    return 1;
}

static int vec3_ctor(lua_State* L)
{
    Stack s(L);
    Vec3r v(s.get_double(1), s.get_double(2), s.get_double(3));
    s.push_vec3(v);
    return 1;
}

static int vec3_tostring(lua_State* L)
{
    Stack s(L);
    Vec3r v = s.get_vec3(1);
    std::ostringstream oss;
    oss << v;
    s.push_string(oss.str().c_str());
    return 1;
}

static int vec3_add(lua_State* L)
{
    Stack s(L);
    Vec3r v1 = s.get_vec3(1);
    Vec3r v2 = s.get_vec3(2);
    s.push_vec3(v1 + v2);
    return 1;
}

static int vec3_sub(lua_State* L)
{
    Stack s(L);
    Vec3r v1 = s.get_vec3(1);
    Vec3r v2 = s.get_vec3(2);
    s.push_vec3(v1 - v2);
    return 1;
}

static int vec3_mul(lua_State* L)
{
    Stack s(L);
    Vec3r v = s.get_vec3(1);
    double x = s.get_double(2);
    s.push_vec3(v * x);
    return 1;
}

static int vec3_length(lua_State* L)
{
    Stack s(L);
    Vec3r v = s.get_vec3(1);
    s.push_double(v.length());
    return 1;
}

static int vec3_normalize(lua_State* L)
{
    Stack s(L);
    Vec3r v = s.get_vec3(1);
    s.push_vec3(v.normalize());
    return 1;
}

static int vec3_cross(lua_State* L)
{
    Stack s(L);
    Vec3r v1 = s.get_vec3(1);
    Vec3r v2 = s.get_vec3(2);
    s.push_vec3(cross(v1, v2));
    return 1;
}

static int bbox_ctor(lua_State* L)
{
    Stack s(L);
    Vec3r pmin = s.get_vec3(1);
    Vec3r pmax = s.get_vec3(2);
    s.push_bbox(BBoxr(pmin, pmax));
    return 1;
}

static int bbox_merge(lua_State* L)
{
    Stack s(L);
    BBoxr b1 = s.get_bbox(1);
    BBoxr b2 = s.get_bbox(2);
    s.push_bbox(merge(b1, b2));
    return 1;
}

static int bbox_min(lua_State* L)
{
    Stack s(L);
    BBoxr b = s.get_bbox(1);
    s.push_vec3(b.pmin);
    return 1;
}

static int bbox_max(lua_State* L)
{
    Stack s(L);
    BBoxr b = s.get_bbox(1);
    s.push_vec3(b.pmax);
    return 1;
}

static int bbox_centroid(lua_State* L)
{
    Stack s(L);
    BBoxr b = s.get_bbox(1);
    s.push_vec3(b.get_centroid());
    return 1;
}

static int bbox_tostring(lua_State* L)
{
    Stack s(L);
    BBoxr b = s.get_bbox(1);
    std::ostringstream oss;
    oss << b;
    s.push_string(oss.str().c_str());
    return 1;
}

static int transform_ctor(lua_State* L)
{
    Stack s(L);
    Transformr xfm;
    s.push_transform(xfm);
    return 1;
}

static int transform_tostring(lua_State* L)
{
    Stack s(L);
    Transformr xfm = s.get_transform(1);
    std::ostringstream oss;
    oss << '\n' << xfm.m;
    s.push_string(oss.str().c_str());
    return 1;
}

static int transform_inverse(lua_State* L)
{
    Stack s(L);
    Transformr xfm = s.get_transform(1);
    Transformr inv(xfm.inv, xfm.m);
    s.push_transform(inv);
    return 1;
}

static int transform_mul(lua_State* L)
{
    Stack s(L);
    Transformr t1 = s.get_transform(1);
    Transformr t2 = s.get_transform(2);
    s.push_transform(t1 * t2);
    return 1;
}

static int make_translation_transform(lua_State* L)
{
    Stack s(L);
    Transformr xfm = make_translation(Vec3r(s.get_double(1), s.get_double(2), s.get_double(3)));
    s.push_transform(xfm);
    return 1;
}

static int make_lookat_transform(lua_State* L)
{
    Stack s(L);
    Vec3r eye = s.get_vec3(1);
    Vec3r target = s.get_vec3(2);
    Vec3r up = s.get_vec3(3);
    Transformr xfm = make_lookat(eye, target, up);
    s.push_transform(xfm);
    return 1;
}

static int make_scale_transform(lua_State* L)
{
    Stack s(L);
    Transformr xfm = make_scale(Vec3r(s.get_double(1), s.get_double(2), s.get_double(3)));
    s.push_transform(xfm);
    return 1;
}

static int make_rotation_x_transform(lua_State* L)
{
    Stack s(L);
    Transformr xfm = make_rotation_x(s.get_double(1));
    s.push_transform(xfm);
    return 1;
}

static int make_rotation_y_transform(lua_State* L)
{
    Stack s(L);
    Transformr xfm = make_rotation_y(s.get_double(1));
    s.push_transform(xfm);
    return 1;
}

static int make_rotation_z_transform(lua_State* L)
{
    Stack s(L);
    Transformr xfm = make_rotation_z(s.get_double(1));
    s.push_transform(xfm);
    return 1;
}

static int make_rotation_transform(lua_State* L)
{
    Stack s(L);
    Transformr xfm = make_rotation(s.get_vec3(1), s.get_double(2));
    s.push_transform(xfm);
    return 1;
}

static int shape_get_bbox(lua_State* L)
{
    Stack s(L);
    ShapeID id = s.get_shape(1);
    s.push_bbox(ShapeManager::get<Shape>(id)->get_bbox());
    return 1;
}

static int world_ctor(lua_State* L)
{
    Stack s(L);
    auto world = std::make_shared<World>();
    s.push_world(world);
    return 1;
}

static int world_dtor(lua_State* L)
{
    Stack s(L);
    auto world = s.get_world(1);
    destroy(world);
    return 0;
}

static int world_add_shape(lua_State* L)
{
    Stack s(L);
    auto world = s.get_world(1);
    ShapeID shape = s.get_shape(2);
    world->add_shape(shape);
    return 0;
}

static int world_get_bbox(lua_State* L)
{
    Stack s(L);
    auto world = s.get_world(1);
    s.push_bbox(world->get_bbox());
    return 1;
}

static int world_preprocess(lua_State* L)
{
    Stack s(L);
    auto world = s.get_world(1);
    world->preprocess();
    return 0;
}

static int make_instance(lua_State* L)
{
    Stack s(L);
    ShapeID id = s.get_shape(1);
    Transformr xfm = s.get_transform(2);
    ShapeID inst_id = ShapeManager::create<ShapeInstance>(id, xfm);
    s.push_shape(inst_id);
    return 1;
}

static int camera_make_perspective(lua_State* L)
{
    Stack s(L);
    luaL_checktype(L, 1, LUA_TTABLE);
    Vec3r eye = safe_getfield_vec3(L, 1, "eye", Vec3r(0, 0, 1));
    Vec3r target = safe_getfield_vec3(L, 1, "target", Vec3r(0, 0, 0));
    Vec3r up = safe_getfield_vec3(L, 1, "up", Vec3r(0, 1, 0));
    double w = safe_getfield_double(L, 1, "frame_width", 512.0);
    double h = safe_getfield_double(L, 1, "frame_height", 512.0);
    double fov = safe_getfield_double(L, 1, "fov", 45.0);
    double lensr = safe_getfield_double(L, 1, "lens_radius", 0.5);
    double dist = safe_getfield_double(L, 1, "focal_distance", 1.0);

    std::shared_ptr<PerspectiveCamera> cam =
        std::make_shared<PerspectiveCamera>(eye, target, up, Vec2u(w, h), fov, lensr, dist);
    s.push_camera(std::dynamic_pointer_cast<Camera>(cam));

    return 1;
}

static int camera_dtor(lua_State* L)
{
    Stack s(L);
    auto camera = s.get_camera(1);
    destroy(camera);
    return 0;
}


static int renderer_ctor(lua_State* L)
{
    Stack s(L);
    std::shared_ptr<World> world = s.get_world(1);
    std::shared_ptr<Camera> cam = s.get_camera(2);
    luaL_checktype(L, 3, LUA_TTABLE);
    int fw = safe_getfield_int(L, 3, "frame_width", 512);
    int fh = safe_getfield_int(L, 3, "frame_height", 512);
    int tw = safe_getfield_int(L, 3, "tile_width", 16);
    int th = safe_getfield_int(L, 3, "tile_height", 16);
    int spp = safe_getfield_int(L, 3, "spp", 10);
    int preview_spp = safe_getfield_int(L, 3, "preview_spp", 1);
    bool preview = safe_getfield_bool(L, 3, "preview", true);
    int adaptive_spp = safe_getfield_int(L, 3, "adaptive_spp", 0);
    int firefly_spp = safe_getfield_int(L, 3, "firefly_spp", 0);
    double adaptive_threshold = safe_getfield_double(L, 3, "adaptive_threshold", 1.0);
    double adaptive_exponent = safe_getfield_double(L, 3, "adaptive_exponent", 1.0);
    double firefly_threshold = safe_getfield_double(L, 3, "firefly_threshold", 1.0);
    const char* tonemap_str = safe_getfield_string(L, 3, "tonemap", "gamma");

    Options opts;
    opts.frame_size = Vec2u(fw, fh);
    opts.tile_size = Vec2u(tw, th);
    opts.spp = spp;
    opts.preview_spp = preview_spp;
    opts.preview = preview;
    opts.adaptive_spp = adaptive_spp;
    opts.adaptive_threshold = adaptive_threshold;
    opts.adaptive_exponent = adaptive_exponent;
    opts.firefly_spp = firefly_spp;
    opts.firefly_threshold = firefly_threshold;
    opts.tonemap = tonemap_from_string(tonemap_str);

    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(world, cam, opts);
    renderer->set_lua_environment(g_environment);
    s.push_renderer(renderer);

    return 1;
}

static int renderer_dtor(lua_State* L)
{
    Stack s(L);
    auto renderer = s.get_renderer(1);
    destroy(renderer);
    return 0;
}

static int renderer_render_interactive(lua_State* L)
{
    Stack s(L);
    auto renderer = s.get_renderer(1);
    renderer->render(true);
    return 0;
}

static int renderer_reset(lua_State* L)
{
    Stack s(L);
    auto renderer = s.get_renderer(1);
    renderer->reset();
    return 0;
}

static int get_path(lua_State* L)
{
    Stack s(L);
    s.push_string(g_path.c_str());
    return 1;
}

void load_api(Environment& env, const std::string& path)
{
    g_environment = &env;
    g_path = path;

    env.register_function("get_path", get_path);

    env.register_function("load_obj", load_obj);

    const luaL_Reg vec3_funcs[] = {
        { "new",        vec3_ctor },
        { "__tostring", vec3_tostring },
        { "__add",      vec3_add },
        { "__sub",      vec3_sub },
        { "__mul",      vec3_mul },
        { "length",     vec3_length },
        { "normalize",  vec3_normalize },
        { "cross",      vec3_cross },
        { nullptr,      nullptr }
    };
    env.register_module("Vec3", vec3_funcs);

    const luaL_Reg bbox_funcs[] = {
        { "new",          bbox_ctor },
        { "merge",        bbox_merge },
        { "min",          bbox_min },
        { "max",          bbox_max },
        { "get_centroid", bbox_centroid },
        { "__tostring",   bbox_tostring },
        { nullptr,        nullptr }
    };
    env.register_module("BBox", bbox_funcs);

    const luaL_Reg transform_funcs[] = {
        { "new",        transform_ctor },
        { "ident",      transform_ctor },
        { "__tostring", transform_tostring },
        { "__mul",      transform_mul },
        { "inverse",    transform_inverse },
        { nullptr,      nullptr }
    };
    env.register_module("Transform", transform_funcs);

    const luaL_Reg shape_funcs[] = {
        { "get_bbox",   shape_get_bbox },
        { nullptr,      nullptr }
    };
    env.register_module("Shape", shape_funcs);

    env.register_function("make_rotation_x", make_rotation_x_transform);
    env.register_function("make_rotation_y", make_rotation_y_transform);
    env.register_function("make_rotation_z", make_rotation_z_transform);
    env.register_function("make_rotation", make_rotation_transform);
    env.register_function("make_scale", make_scale_transform);
    env.register_function("make_translation", make_translation_transform);
    env.register_function("make_lookat", make_lookat_transform);

    const luaL_Reg world_funcs[] = {
        { "new",        world_ctor },
        { "__gc",       world_dtor },
        { "add_shape",  world_add_shape },
        { "get_bbox",   world_get_bbox },
        { "preprocess", world_preprocess },
        { nullptr,      nullptr }
    };
    env.register_module("World", world_funcs);

    env.register_function("make_instance", make_instance);

    const luaL_Reg camera_funcs[] = {
        { "make_perspective", camera_make_perspective },
        { "__gc",             camera_dtor },
        { nullptr,            nullptr }
    };
    env.register_module("Camera", camera_funcs);

    const luaL_Reg renderer_funcs[] = {
        { "new",                renderer_ctor },
        { "__gc",               renderer_dtor },
        { "render_interactive", renderer_render_interactive },
        { "reset",              renderer_reset },
        { nullptr,              nullptr }
    };
    env.register_module("Renderer", renderer_funcs);
}

} } // namespace hop::lua
