#include "mouse.hpp"

#include "../wiimote/wiimote.hpp"
#include "lua.h"
#include "sol/state_view.hpp"

int lua_mouse_getPosition(lua_State* L)
{
  double pos_x = -1; 
  double pos_y = -1;

  love::wiimote::WiimoteController* remote = love::wiimote::getWiimote(1);

  if(remote && remote->data && remote->data->ir.valid)
  {
    pos_x = remote->getSmoothX();
    pos_y = remote->getSmoothY();
  }

  lua_pushnumber(L, pos_x);
  lua_pushnumber(L, pos_y);
  return 2;
}


int luaopen_love_mouse(lua_State* L)
{
  printf("<== MODULE LOVE MOUSE ==>");
  sol::state_view lstate(L);
  lstate["love"]["mouse"] = lstate.create_table_with(
        "getPosition", lua_mouse_getPosition
      );
  return 1;
}
