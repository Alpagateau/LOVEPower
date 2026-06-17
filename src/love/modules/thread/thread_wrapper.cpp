
#include "thread_wrapper.h"
#include "SDL/SDL_mutex.h"
#include "sol/state_view.hpp"
#include <map>
#include <string>
extern "C" {
  #include <lauxlib.h>
  #include <lua.h>
}
#include "thread.h"
#include <cstring>

static const luaL_reg *global_modules_list = NULL;

love::thread::mutex* global_mutex;
std::map<std::string, love_channel_t*> global_channels;

int lua_safeprint(lua_State* L)
{
  lock(global_mutex);
  const char* str = lua_tostring(L, 1);
  printf("%s\n", str);
  fflush(stdout);
  unlock(global_mutex);
  return 0;
}

variant_t* to_variant(lua_State* L, int idx)
{
  int t = lua_type(L, idx);
  variant_t* v = (variant_t*)malloc(sizeof(*v));
  switch(t)
  {
    case LUA_TNUMBER:
      v->kind = VARIANT_NUMBER;
      v->data.number = lua_tonumber(L, idx);
      break;
    case LUA_TBOOLEAN:
      v->kind = VARIANT_BOOLEAN;
      v->data.boolean = lua_toboolean(L, idx);
      break; 
    case LUA_TNIL:
      v->kind = VARIANT_NIL;
      break;
    case LUA_TSTRING:
      v->kind = VARIANT_STRING;
      v->data.str = strdup(lua_tostring(L, idx));
      break;
  }
  return v;
}

int lua_pushvariant(lua_State* L, variant_t* v)
{
  if(v == NULL){
    lua_pushnil(L);
    return 1; 
  }
  switch (v->kind) {
    case VARIANT_BOOLEAN:
      lua_pushboolean(L, v->data.boolean);
      break;
    case VARIANT_NUMBER:
      lua_pushnumber(L, v->data.number);
      break;
    case VARIANT_NIL:
      lua_pushnil(L);
      break;
    case VARIANT_STRING:
      lua_pushstring(L, v->data.str);
      break;
    case VARIANT_USERDATA:
      lua_pushlightuserdata(L, v->data.ud);
      break;
  }
  return 1;
}

int love_thread_newThread(lua_State *L) {
  const char *arg = luaL_checkstring(L, -1);
  bool is_path = true;
  if (strchr(arg, ';') || strchr(arg, '\n') || strchr(arg, '='))
    is_path = false;

  love_thread_t *t = (love_thread_t *)malloc(sizeof(love_thread_t));
  *t = love::thread::newThread(arg, is_path);

  love_thread_t **udata =
      (love_thread_t **)lua_newuserdata(L, sizeof(love_thread_t *));
  *udata = t;

  luaL_getmetatable(L, LUA_THREAD_META);
  lua_setmetatable(L, -2);

  return 1;
}

love_thread_t *to_thread(lua_State *L, int idx) {
  love_thread_t **t_ptr =
      (love_thread_t **)luaL_checkudata(L, idx, LUA_THREAD_META);
  return *t_ptr;
}

int thread_start(lua_State *L) {
  love_thread_t *t = to_thread(L, 1);
  love::thread::runThread(t, global_modules_list);
  return 0;
}

int thread_isRunning(lua_State *L) {
  love_thread_t *t = to_thread(L, 1);
  lua_pushboolean(L, t->running);
  return 1;
}

int thread_getError(lua_State *L) {
  love_thread_t *t = to_thread(L, 1);
  if (t->error == NULL) {
    lua_pushnil(L);
  } else {
    lua_pushstring(L, strdup(t->error));
  }
  return 1;
}

int thread_gc(lua_State *L) {
  love_thread_t *t = to_thread(L, 1);

  if (t->handle && t->running) {
    SDL_WaitThread(t->handle, NULL);
  }

  // Clean up allocated buffer strings
  if (t->buffer)
    free((void *)t->buffer);
  if (t->error)
    free(t->error);

  free(t);
  return 0;
}

int set_global_modules(const luaL_reg* modules)
{
  if(global_modules_list == NULL)
    global_modules_list = modules;

  return 0;
}

extern "C" int luaopen_threads(lua_State *L) {
  if (global_mutex == NULL)
  {
    global_mutex = SDL_CreateMutex();
  }

  printf("<== LOVE THREADS ==>\n");

  // 2. Create and set up the Thread Object Metatable
  luaL_newmetatable(L, LUA_THREAD_META);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_register(L, NULL, thread_methods);
  lua_pop(L, 1);

  luaL_newmetatable(L, LUA_CHANNEL_META);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_register(L, NULL, channel_methods);
  lua_pop(L, 1);

  sol::state_view ls(L);

  ls["love"]["thread"] = ls.create_table_with(
        "sprint", lua_safeprint,
        "newThread", love_thread_newThread,
        "newChannel", love_thread_newChannel,
        "getChannel", love_thread_getChannel
      );

  //lua_getglobal(L, "love");
  //if (lua_isnil(L, -1)) {
  //  lua_pop(L, 1);
  //  lua_newtable(L);
  //  lua_setglobal(L, "love");
  //  lua_getglobal(L, "love");
  //}

  //lua_newtable(L);
  //luaL_register(L, NULL, thread_functions);
  //lua_setfield(L, -2, "thread"); // love.thread = module table
 
  //lua_pop(L, 1); // Pop love table
  return 1;
}

int love_thread_newChannel(lua_State* L)
{
  love_channel_t* ch = channel_new();
  love_channel_t** udata =  (love_channel_t**)lua_newuserdata(L, sizeof(love_channel_t*));
  *udata = ch;
  luaL_getmetatable(L, LUA_CHANNEL_META);
  lua_setmetatable(L, -2);
  return 1;
}

int love_thread_getChannel(lua_State* L)
{
  love_channel_t* c = NULL;
  std::string name = lua_tostring(L, -1);
  lock(global_mutex);
  if(auto ch = global_channels.find(name); ch != global_channels.end()){
    c = ch->second;
  }
  else {
    c = channel_new();
    global_channels[name] = c;
  }
  unlock(global_mutex);
  love_channel_t** udata =  (love_channel_t**)lua_newuserdata(L, sizeof(love_channel_t*));
  *udata = c;
  luaL_getmetatable(L, LUA_CHANNEL_META);
  lua_setmetatable(L, -2);
  return 1;
}

love_channel_t* to_channel(lua_State* L, int idx)
{
  love_channel_t **ch_ptr =
      (love_channel_t **)luaL_checkudata(L, idx, LUA_CHANNEL_META);
  return *ch_ptr;
}

int channel_lua_push(lua_State* L)
{
  love_channel_t* ch = to_channel(L, 1);
  variant_t* v = to_variant(L, 2);
  channel_push(ch, v);
  free(v);
  return 0;
}

int channel_lua_pop(lua_State* L)
{
  love_channel_t* ch = to_channel(L, 1);
  variant_t v = {};
  if (channel_pop(ch, &v)){;
    lua_pushvariant(L, &v); 
    if(v.kind == VARIANT_STRING && v.data.str)
      free(v.data.str);
  }
  else
    lua_pushnil(L);
  return 1;
}

int channel_lua_demand(lua_State* L)
{
  love_channel_t* ch = to_channel(L, 1);
  variant_t v = {};
  channel_demand (ch, &v);
  lua_pushvariant(L, &v); 
  if(v.kind == VARIANT_STRING && v.data.str)
    free(v.data.str);
  return 1;
}

int channel_lua_getCount(lua_State* L)
{
  love_channel_t* ch = to_channel(L, 1);
  int value = channel_getCount(ch);
  lua_pushnumber(L, (double)value);
  return 1;
}

int channel_lua_clear   (lua_State* L)
{
  love_channel_t* ch = to_channel(L, 1);
  channel_clear(ch);
  return 0;
}
