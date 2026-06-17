#ifndef JOYSTICK_WRAPPER_H
#define JOYSTICK_WRAPPER_H

#include "joystick.h"

extern "C" {
  #include <lua.h>
}

extern "C" int luaopen_love_joystick(lua_State* L);

#endif
