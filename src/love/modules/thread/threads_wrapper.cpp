#include "threads_wrapper.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "sdl/threads_sdl.h"
#include "sol/state_view.hpp"
#include "threads_love.h"

#define THREAD_METATABLE "love.types.thread"

int luaopen_love_threads(lua_State* L)
{
  sol::state_view luastate(L);
  luastate["love"]["thread"] = luastate.create_table_with(
      "getChannel", love::thread::getChannel,
      "newThread", love::thread::newNamedThread,
      "newChannel", love::thread::newChannel
    );
  
  luaL_newmetatable(L, THREAD_METATABLE); 
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2);
  lua_settable(L, -3);

  static const luaL_reg thread_meta [] = {
    //{"getError" , NULL},
    {"isRunning", love::thread::wrap_isRunning},
    {"start"    , love::thread::wrap_start},
    {"wait"     , love::thread::wrap_wait},
    {NULL, NULL}
  };
  luaL_openlib(L, NULL, thread_meta, 0);

  return 1;
}

int love::thread::newNamedThread(lua_State *L)
{
  if(!lua_isstring(L, -1)) 
  {
    printf("[LOVE THREADS] Only file paths are accepted to create threads.\n");
    lua_pop(L, 1);
    lua_pushnil(L);
    return 1;
  }

  std::string path = lua_tostring(L, -1);
  lua_pop(L, 1);

  love::thread::sdl1::LWPThread* t = 
    (love::thread::sdl1::LWPThread*)lua_newuserdata(L, sizeof(sdl1::LWPThread));
  
  if(t == NULL){
    printf("[LOVE THREADS] Coulndt create the userdata\n");
    lua_pushnil(L);
    return 1;
  }
 
  love::thread::Threadable e;
  *t = sdl1::LWPThread(NULL);
  t->t = &e;
  //t->t = [](){};
  luaL_getmetatable(L, THREAD_METATABLE);
  lua_setmetatable(L, -2); 
  return 1;
}

int love::thread::wrap_start(lua_State *L)
{
  void* t = luaL_checkudata(L, -1, THREAD_METATABLE);
  if(t == NULL)
    return 0;
  love::thread::sdl1::LWPThread* th = (love::thread::sdl1::LWPThread*)t;
  th->start();
  return 0;
}


int love::thread::wrap_wait(lua_State *L)
{
  void* t = luaL_checkudata(L, -1, THREAD_METATABLE);
  if(t == NULL)
    return 0;
  love::thread::sdl1::LWPThread* th = (love::thread::sdl1::LWPThread*)t;
  th->wait();
  return 0;
}


int love::thread::wrap_isRunning(lua_State *L)
{
  void* t = luaL_checkudata(L, -1, THREAD_METATABLE);
  if(t == NULL)
    return 0;
  love::thread::sdl1::LWPThread* th = (love::thread::sdl1::LWPThread*)t;
  bool r = th->isRunning();
  lua_pushboolean(L, r);
  return 1;
}
