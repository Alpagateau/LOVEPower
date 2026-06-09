#ifndef THREADS_WRAPPER_H
#define THREADS_WRAPPER_H

#include <lua.h>
int luaopen_love_threads();

namespace love 
{
  namespace thread
  {
    int newNamedThread(lua_State* L);
    int getChannel(lua_State* L);
    int newChannel(lua_State* L);

    int wrap_start(lua_State* L);
    int wrap_wait(lua_State* L);
    int wrap_isRunning(lua_State* L);

  };
};

#endif
