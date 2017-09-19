#pragma once

#include "lua/lua.h"

#include "math/vec3.h"
#include "math/transform.h"
#include "geometry/world.h"
#include "camera/camera.h"
#include "render/renderer.h"

#include <memory>

namespace hop { namespace lua {

class Stack
{
public:
    Stack(lua_State* L) : L(L) { }

    void push_int(int value)
    {
        lua_pushinteger(L, value);
    }

    void push_bool(bool value)
    {
        lua_pushboolean(L, value);
    }

    void push_double(double value)
    {
        lua_pushnumber(L, value);
    }

    void push_string(const char* s)
    {
        lua_pushstring(L, s);
    }

    void push_vec3(const Vec3r& v)
    {
        Vec3r* v_ptr = (Vec3r*)lua_newuserdata(L, sizeof(Vec3r));
        *v_ptr = v;
        luaL_getmetatable(L, "Vec3");
        lua_setmetatable(L, -2);
    }

    void push_transform(const Transformr& xfm)
    {
        Transformr* t_ptr = (Transformr*)lua_newuserdata(L, sizeof(Transformr));
        *t_ptr = xfm;
        luaL_getmetatable(L, "Transform");
        lua_setmetatable(L, -2);
    }

    void push_world(std::shared_ptr<World> world)
    {
        using WorldPtr = std::shared_ptr<World>;
        auto userdata = static_cast<WorldPtr*>(lua_newuserdata(L, sizeof(WorldPtr)));
        new (userdata) WorldPtr(world);
        luaL_getmetatable(L, "World");
        lua_setmetatable(L, -2);
    }

    void push_camera(std::shared_ptr<Camera> cam)
    {
        using CamPtr = std::shared_ptr<Camera>;
        auto userdata = static_cast<CamPtr*>(lua_newuserdata(L, sizeof(CamPtr)));
        new (userdata) CamPtr(cam);
        luaL_getmetatable(L, "Camera");
        lua_setmetatable(L, -2);
    }

    void push_renderer(std::shared_ptr<Renderer> renderer)
    {
        using RPtr = std::shared_ptr<Renderer>;
        auto userdata = static_cast<RPtr*>(lua_newuserdata(L, sizeof(RPtr)));
        new (userdata) RPtr(renderer);
        luaL_getmetatable(L, "Renderer");
        lua_setmetatable(L, -2);
    }

    int get_int(int i)
    {
        return luaL_checknumber(L, i);
    }

    int get_bool(int i)
    {
        return lua_toboolean(L, i) == 1;
    }

    double get_double(int i)
    {
        return luaL_checknumber(L, i);
    }

    const char* get_string(int i)
    {
        return luaL_checkstring(L, i);
    }

    Vec3r get_vec3(int i)
    {
        return *((Vec3r*)luaL_checkudata(L, i, "Vec3"));
    }

    Transformr get_transform(int i)
    {
        return *((Transformr*)luaL_checkudata(L, i, "Transform"));
    }

    std::shared_ptr<World> get_world(int i)
    {
        return *static_cast<std::shared_ptr<World>*>(luaL_checkudata(L, i, "World"));
    }

    std::shared_ptr<Camera> get_camera(int i)
    {
        return *static_cast<std::shared_ptr<Camera>*>(luaL_checkudata(L, i, "Camera"));
    }

    std::shared_ptr<Renderer> get_renderer(int i)
    {
        return *static_cast<std::shared_ptr<Renderer>*>(luaL_checkudata(L, i, "Renderer"));
    }

    void pop(int n)
    {
        lua_pop(L, n);
    }

private:
    lua_State* L;
};

} } // namespace hop::lua
