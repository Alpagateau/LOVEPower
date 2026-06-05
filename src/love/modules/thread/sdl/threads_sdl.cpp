#include <cstdio>
#include <gccore.h>
#include <malloc.h>
#include "../threads_love.h"
#include "../Thread.h"
#include <SDL/SDL.h>
#include <ogc/mutex.h>
#include <ogc/cond.h>

namespace love {
namespace thread {
namespace sdl1 {

// Derived Mutex for SDL1
struct LWPMutex : public Mutex {
    //SDL_mutex* m;
    mutex_t m;

    LWPMutex() { 
      //m = SDL_CreateMutex(); 
      s32 v = LWP_MutexInit(&m, false);
      if(v < 0)
        printf("Couldnt create a mutex");
    }
    ~LWPMutex() { 
      //SDL_DestroyMutex(m); 
      LWP_MutexDestroy(m);
    }

    void lock() { 
      //SDL_mutexP(m); 
      LWP_MutexLock(m);
    }
    void unlock() { 
      //SDL_mutexV(m); 
      LWP_MutexUnlock(m);
    }
};

// Derived Conditional for SDL1
struct LWPConditional : public Conditional {
    cond_t c;

    LWPConditional() { 
      LWP_CondInit(&c);
    }
    ~LWPConditional() { 
      LWP_CondDestroy(c);
    }

    void signal() override { 
      //SDL_CondSignal(c); 
      LWP_CondSignal(c);
    }
    void broadcast() override { 
      //SDL_CondBroadcast(c); 
      LWP_CondBroadcast(c);
    }

    bool wait(Mutex* mutex, int timeout=-1) override {
        if (!mutex) return false;
        mutex_t mtx = static_cast<LWPMutex*>(mutex)->m;
        if (timeout < 0) return LWP_CondWait(c, mtx) == 0;
        else return LWP_CondWait(c, mtx) == 0;
    }
};

struct LWPThread : public Thread {
    lwp_t thread;
    uint8_t* stack;
    Threadable* t;
    bool running;

    LWPThread(Threadable* t) : thread(LWP_THREAD_NULL), t(t), running(false) {
        // Allocate a 128KB stack for Lua, aligned to 32 bytes (crucial for Wii)
        stack = (uint8_t*)memalign(32, 128 * 1024);
    }

    ~LWPThread() override {
        if (stack) free(stack);
    }

    bool start() override {
        if (running) return false;
        
        running = true;
        // Priority 64 is middle-of-the-road. 
        int res = LWP_CreateThread(&thread, thread_runner, this, stack, 128 * 1024, 64);
        if (res != 0) {
            running = false;
            return false;
        }
        return true;
    }

    void wait() override {
        if (thread != LWP_THREAD_NULL) {
            LWP_JoinThread(thread, nullptr);
            thread = LWP_THREAD_NULL;
        }
    }

    bool isRunning() override {
        return running;
    }

private:
    static void* thread_runner(void* data) {
        LWPThread* self = static_cast<LWPThread*>(data);
        self->t->threadFunction();
        self->running = false; // Accurately mark as finished!
        return nullptr;
    }
};

} // namespace sdl1

// Factory functions
Mutex* newMutex() { return new sdl1::LWPMutex(); }
Conditional* newConditional() { return new sdl1::LWPConditional(); }
//Thread* newThread(Threadable* t) { return new sdl1::LWPThread(t); }

Thread* newThread(Threadable* t) { return new sdl1::LWPThread(t); }

} // namespace thread
} // namespace love
