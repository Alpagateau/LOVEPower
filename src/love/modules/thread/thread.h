#ifndef THREAD_H
#define THREAD_H

#include <queue>

extern "C" {
  #include "SDL/SDL_mutex.h"
  #include "SDL/SDL_thread.h"
  #include <lauxlib.h>
  #include <lua.h>
}

namespace love{
namespace thread{

typedef SDL_mutex mutex;

enum variant_type
{
  VARIANT_NIL,  
  VARIANT_NUMBER,
  VARIANT_BOOLEAN,
  VARIANT_STRING,
  VARIANT_USERDATA
};

typedef struct {
  enum variant_type kind;
  union {
    double number;
    bool boolean;
    char* str;
    void* ud;
  } data;
}variant_t;

typedef struct{
  SDL_Thread* handle;
  char* buffer; //code
  //bool is_path; //true if buffer is filename
  volatile bool running; 
  char* error;
  lua_State* L;
} love_thread_t;

typedef struct{
  love_thread_t* thread;
  const luaL_reg* modules;
}love_thread_data_t;

typedef struct{
  mutex* m;
  SDL_cond* cond;
  std::queue<variant_t> messages;
}love_channel_t;

//variant helpers
void free_variant(variant_t* v);

//mutex functions
mutex* newMutex();
void lock(mutex* m);
void unlock(mutex* m);

//thread functions
love_thread_t newThread(const char* b, bool is_path); 
void          runThread(love_thread_t* thread, const luaL_reg* modules);

//channel functions
love_channel_t* channel_new();
void channel_push    (love_channel_t* ch, variant_t* v);
bool channel_pop     (love_channel_t* ch, variant_t* out);
void channel_demand  (love_channel_t* ch, variant_t* out);
int  channel_getCount(love_channel_t* ch);
void channel_clear   (love_channel_t* ch);
void channel_free    (love_channel_t* ch);

}}
#endif
