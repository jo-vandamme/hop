#include "lua/environment.h"
#include "util/log.h"

namespace hop { namespace lua {

extern void load_api(Environment& env);

void check_errors(lua_State* L, int status)
{
    if (!L) return;

    if (status != 0)
    {
        Log("lua") << WARNING << "Lua: " << lua_tostring(L, -1);
        lua_pop(L, 1);
    }
}

int lua_print_func(lua_State* L)
{
    const char* str = luaL_checkstring(L, 1);
    Log("lua") << INFO << str;
    return 0;
}

static int error_handler(lua_State* L)
{
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_pushvalue(L, 1); // push error message
    lua_pushinteger(L, 2); // skip this function and traceback
    lua_call(L, 2, 1); // call debug.traceback

    Log("lua") << ERROR << "Lua error: " << lua_tostring(L, -1);

    lua_pop(L, 1); // remove error message from the stack
    lua_pop(L, 1); // remove debug.traceback from the stack

    return 0;
}

Environment::Environment()
{
    L = luaL_newstate();

    luaopen_io(L);
    luaopen_base(L);
    luaopen_table(L);
    luaopen_string(L);
    luaopen_math(L);
    luaL_openlibs(L);
}

Environment::~Environment()
{
    if (L)
        lua_close(L);
}

void Environment::load(const char* file)
{
    int status = luaL_loadfile(L, file);

    lua_pushcfunction(L, lua_print_func);
    lua_setglobal(L, "print");

    load_api(*this);

    if (status == 0)
        lua_pcall(L, 0, 0, 0);

    check_errors(L, status);
}

void Environment::execute_string(const char* s)
{
    lua_pushcfunction(L, error_handler);
    luaL_loadstring(L, s);
    lua_pcall(L, 0, 0, -2);
    lua_pop(L, 1);
}

void Environment::execute(const char* func)
{
    lua_pushcfunction(L, error_handler);
    lua_getglobal(L, func);
    lua_pcall(L, 0, 0, -2);
    lua_pop(L, 1);
}

void Environment::execute(const char* func, int arg)
{
    lua_getglobal(L, func);
    lua_pushinteger(L, arg);
    lua_pcall(L, 1, 0, -2);
    lua_pop(L, 1);
}

void Environment::register_module(const char* module, const luaL_Reg* funcs)
{
    luaL_newmetatable(L, module);
#if LUA_VERSION_NUM >= 502
    luaL_setfuncs(L, funcs, 0);
#else
    luaL_register(L, module, funcs);
#endif
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    lua_setglobal(L, module);
}

void Environment::register_function(const char* func_name, const lua_CFunction func)
{
    lua_pushcfunction(L, func);
    lua_setglobal(L, func_name);
}

} } // namespace hop::lua
