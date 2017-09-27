#pragma once

#include "lua/lua.h"

namespace hop { namespace lua {

class Environment
{
public:
    Environment();
    ~Environment();

    void load(const char* file);

    void call(const char* func, const char* sig, ...);

    void execute_string(const char* s);

    void register_function(const char* func_name, const lua_CFunction func);
    void register_module(const char* module_name, const luaL_Reg* funcs);

    void set_path(const char* path);

    Environment(const Environment&) = delete;
    Environment& operator=(const Environment&) = delete;

private:
    lua_State* L;
};

} } // namespace hop::lua
