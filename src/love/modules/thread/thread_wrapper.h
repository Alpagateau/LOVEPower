#ifndef THREAD_WRAPPER_H
#define THREAD_WRAPPER_H

#include "thread.h"
extern "C" {
  #include "lua.h"
}

#define LUA_THREAD_META  "LoveThread"
#define LUA_CHANNEL_META "LoveChannel"

using namespace love::thread;

//helpers
extern "C" int lua_safeprint(lua_State* L);

//variants
variant_t* to_variant(lua_State* L, int idx);
int lua_pushvariant(lua_State* L, variant_t* v);

//threads
int love_thread_newThread(lua_State* L);

love_thread_t* to_thread(lua_State* L, int idx);
int thread_start(lua_State* L);
int thread_isRunning(lua_State* L);
int thread_getError(lua_State* L);
int thread_gc(lua_State* L);

//channel
int love_thread_newChannel     (lua_State* L);
int love_thread_getChannel     (lua_State* L);
love_channel_t* to_channel(lua_State* L, int idx);

int channel_lua_push    (lua_State* L);
int channel_lua_pop     (lua_State* L);
int channel_lua_demand  (lua_State* L);
int channel_lua_getCount(lua_State* L);
int channel_lua_clear   (lua_State* L);

static const luaL_reg thread_methods[] = {
  {"start"    , thread_start    },
  {"isRunning", thread_isRunning},
  {"getError" , thread_getError },
  {"__gc"     , thread_gc       },
  {NULL, NULL}
};

static const luaL_reg channel_methods[] = {
  {"push"    , channel_lua_push    },
  {"pop"     , channel_lua_pop     },
  {"demand"  , channel_lua_demand  },
  {"getCount", channel_lua_getCount},
  {"clear"   , channel_lua_clear   },
  {NULL, NULL}
};

static const luaL_reg thread_functions[] = {
    //helpers
    {"sprint", lua_safeprint},
    //thread
    {"newThread", love_thread_newThread},
    //Channels
    {"newChannel", love_thread_newChannel},
    {"getChannel", love_thread_getChannel},
    {NULL, NULL}
};

extern "C" int luaopen_threads(lua_State *L,const luaL_reg* master_modules);

#endif
