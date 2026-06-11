#include "thread.h"
#include "SDL/SDL_mutex.h"
#include "SDL/SDL_thread.h"
#include <cstdlib>
#include <cstring>
#include <ogc/system.h>
#include <queue>

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

namespace love{
namespace thread{

love_thread_t newThread(const char* b, bool is_path)
{
  love_thread_t t = {};
  t.buffer  = b;
  t.is_path = is_path;
  t.running = false;
  t.error = NULL;
  return t;
}

extern "C" int luaopen_threads(lua_State *L, const luaL_reg* master_modules);

int runner(void* data)
{
  SYS_STDIO_Report(true);
  printf("[LOVE THREAD] RUNNER\n");
  love_thread_data_t* d = (love_thread_data_t*)data;
  d->thread->L = luaL_newstate();
  luaL_openlibs(d->thread->L); 
  luaopen_threads(d->thread->L, d->modules);
  printf("opening libs\n");
  //luaL_openlib(d->thread->L,NULL, d->modules, 0);
  d->thread->running = true;

  int status = 0;

  printf("<= running the code =>\n");
  if(d->thread->is_path)
    status = luaL_dofile(d->thread->L, d->thread->buffer);
  else
    status = luaL_dostring(d->thread->L, d->thread->buffer);

  printf("Error ?\n");
  if(status != 0) 
  {
    const char* err_msg = lua_tostring(d->thread->L, -1);
    if(err_msg != NULL)
    {
      d->thread->error = strdup(err_msg);
    }
  }

  printf("Closing\n");
  d->thread->running = false;
  lua_close(d->thread->L);
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
  printf("running the thread\n");
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
  printf("Creating the channel\n");
  love_channel_t* ch = new love_channel_t;
  printf("Creating mutex\n");
  ch->m    = SDL_CreateMutex(); 
  printf("Creating cond\n");
  ch->cond = SDL_CreateCond();
  printf("Creating queue\n");
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
