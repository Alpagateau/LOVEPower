#ifndef LOVE_MOUSE_HPP
#define LOVE_MOUSE_HPP

extern "C" {
#include <lua.h>
}

int lua_mouse_getPosition(lua_State* L);

int luaopen_love_mouse(lua_State* L);

#endif
