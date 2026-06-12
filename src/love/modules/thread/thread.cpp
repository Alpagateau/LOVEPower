#include "thread.h"
#include "SDL/SDL_mutex.h"
#include "SDL/SDL_thread.h"
#include "sol/compatibility/compat-5.3.h"
#include "thread_wrapper.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ogc/system.h>
#include <queue>

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
  #include <stdlib.h>
}

namespace love{
namespace thread{

char* buffer_file(const char* path)
{
  char* text_buffer = NULL;
  FILE* source = fopen(path, "r");
  if(source == NULL)
    printf("Couldnt open the file\n");
  fseek(source, 0, SEEK_END);
  long size = ftell(source) + 1;
  fseek(source, 0, SEEK_SET);
  text_buffer = (char*)malloc(size);
  int idx = 0;
  int c = 0;
  while((c = fgetc(source)) != EOF) {
    text_buffer[idx++] = c;
  }
  text_buffer[idx] = 0;
  printf("Done buffering the file [%zu bytes]\n", strlen(text_buffer));
  fclose(source);
  return text_buffer;
}

love_thread_t newThread(const char* b, bool is_path)
{
  static const char* game_folder = "game/";
  love_thread_t t = {};

  if(is_path){
    char* full_path = (char*)malloc(strlen(b) + strlen(game_folder) + 1);
    sprintf(full_path, "%s%s", game_folder, b);
    t.buffer  = buffer_file(full_path);
    free(full_path);
  }
  else
  {
    t.buffer = strdup(b);
  }
  t.running = false;
  t.error = NULL;
  return t;
}

extern "C" int luaopen_threads(lua_State *L, const luaL_reg* master_modules);
extern "C" int lua_safeprint(lua_State *L);

int runner(void* data)
{
  SYS_STDIO_Report(true);
  love_thread_data_t* d = (love_thread_data_t*)data;
  d->thread->L = luaL_newstate();
  luaL_openlibs(d->thread->L);
  lua_register(d->thread->L, "s_print", lua_safeprint);
  int idx = 0;
  while(d->modules[idx].name != NULL)
  {
    //luaL_requiref(d->thread->L, d->modules[idx].name, d->modules[idx].func, 1);
    
    lua_getglobal(d->thread->L, "package");
    lua_getfield(d->thread->L , -1, "preload");
    lua_pushcfunction(d->thread->L , d->modules[idx].func);
    lua_setfield(d->thread->L, -2, d->modules[idx].name);
    lua_pop(d->thread->L, 2);
    idx++;
  }

  luaopen_threads(d->thread->L, d->modules); 

  lua_getglobal(d->thread->L, "debug");
  lua_getfield(d->thread->L, -1, "traceback");
  lua_remove(d->thread->L, -2); // Remove the 'debug' table, leaving just 'traceback'

  int msg_handler_idx = lua_gettop(d->thread->L);

  d->thread->running = true;
  int status = 0;

  printf("[THREADS] starting the programe (%p)\n", d);
  if(luaL_loadbuffer(d->thread->L, d->thread->buffer, strlen(d->thread->buffer), "threadcode") != 0)
  {
    printf("syntax error : %s\n", lua_tostring(d->thread->L, -1));
    lua_pop(d->thread->L, 1);
  }
  else
    status = lua_pcall(d->thread->L, 0, LUA_MULTRET, msg_handler_idx);
  printf("[THREADS] thread is done (%p)\n", d);

  if(status != 0) 
  {
    const char* err_msg = lua_tostring(d->thread->L, -1);
    if(err_msg != NULL)
    {
      d->thread->error = strdup(err_msg);
      printf("[THREADS] (%p) error : %s\n", d, err_msg);
    }
  }

  d->thread->running = false;
  lua_close(d->thread->L);
  free(d->thread->buffer);
  free(d);
  return 0;
}

void runThread(love_thread_t* thread, const luaL_reg* modules)
{
  if(thread->running) return;
  if(thread->error != NULL)
  {
    free(thread->error);
    thread->error = NULL;
  }
  love_thread_data_t* data = (love_thread_data_t*)malloc(sizeof(*data));
  data->thread = thread;
  data->modules = modules;
  thread->handle = SDL_CreateThread(runner, data);
}

mutex* newMutex()
{
  return SDL_CreateMutex();
}

void lock(mutex* m)
{
  SDL_LockMutex(m);
}

void unlock(mutex* m)
{
  SDL_UnlockMutex(m);
}

void free_variant(variant_t* v)
{
  switch (v->kind) {
    case VARIANT_NIL:
    case VARIANT_NUMBER:
    case VARIANT_BOOLEAN:
      break;
    case VARIANT_STRING:
      free(v->data.str);
      break;
    case VARIANT_USERDATA:
      free(v->data.ud);
      break;
  }
  free(v);
}


love_channel_t* channel_new()
{
  love_channel_t* ch = new love_channel_t;
  ch->m    = SDL_CreateMutex(); 
  ch->cond = SDL_CreateCond();
  ch->messages = std::queue<variant_t>();
  return ch;
}

void channel_push    (love_channel_t* ch, variant_t* v)
{
  lock(ch->m);
  ch->messages.push(*v);
  SDL_CondSignal(ch->cond);
  unlock(ch->m);
}

bool channel_pop     (love_channel_t* ch, variant_t* out)
{
  lock(ch->m);
  if(ch->messages.empty())
    return false;
  *out = ch->messages.front();
  ch->messages.pop();
  SDL_CondSignal(ch->cond);
  unlock(ch->m);
  return true;
}

void channel_demand  (love_channel_t* ch, variant_t* out)
{
  lock(ch->m);
  while(ch->messages.empty())
  {
    SDL_CondWait(ch->cond, ch->m);
  }
  *out = ch->messages.front();
  ch->messages.pop();
  unlock(ch->m);
}

int  channel_getCount(love_channel_t* ch)
{
  int len;
  lock(ch->m);
  len = ch->messages.size();
  unlock(ch->m);
  return len;
}

void channel_clear   (love_channel_t* ch)
{
  lock(ch->m);
  while(!ch->messages.empty())
  {
    //not sure about freeing the variants here
    variant_t* v = (variant_t*)malloc(sizeof(variant_t));
    *v = ch->messages.front();
    free_variant(v);
    ch->messages.pop();
  }
  unlock(ch->m);
}

void channel_free    (love_channel_t* ch)
{
  channel_clear(ch);
  SDL_DestroyMutex(ch->m);
  SDL_DestroyCond(ch->cond);
}

}}
