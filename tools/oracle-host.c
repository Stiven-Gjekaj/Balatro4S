// oracle-host: runs a Lua file with the LuaJIT of Balatro.app.
// The LuaJIT of Homebrew uses a fused multiply-add in math.randomseed, and
// gives other random numbers. The LuaJIT of Balatro does not.
// tools/oracle.sh builds and runs it.

#include <stdio.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: oracle-host FILE.lua [ARGS]\n");
        return 2;
    }
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lua_newtable(L);
    for (int i = 1; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i - 1);
    }
    lua_setglobal(L, "arg");
    if (luaL_dofile(L, argv[1])) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        return 1;
    }
    return 0;
}
