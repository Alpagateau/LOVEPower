#pragma once

#include "../threads_love.h"
#include "../Thread.h"
#include <SDL/SDL.h>
#include <ogc/cond.h>
#include <ogc/lwp.h>
#include <ogc/mutex.h>

namespace love {
namespace thread {
namespace sdl1 {

// ------------------------------------------------------------------
// Mutex
// ------------------------------------------------------------------
struct LWPMutex : public Mutex {
    mutex_t* m;

    LWPMutex();
    ~LWPMutex() override;

    void lock() override;
    void unlock() override;
};

// ------------------------------------------------------------------
// Conditional
// ------------------------------------------------------------------
struct LWPConditional : public Conditional {
    cond_t* c;

    LWPConditional();
    ~LWPConditional() override;

    void signal() override;
    void broadcast() override;
    bool wait(Mutex* mutex, int timeout=-1) override;
};

// ------------------------------------------------------------------
// Thread
// ------------------------------------------------------------------
struct LWPThread : public Thread {
    lwp_t* thread;
    uint8_t* stack;
    Threadable* t;
    bool running;

    LWPThread(Threadable* t);
    ~LWPThread() override;

    bool start() override;
    void wait() override;
    bool isRunning() override;

private:
    static int thread_runner(void* data);
};

// ------------------------------------------------------------------
// Factory functions
// ------------------------------------------------------------------
Mutex* newMutex();
Conditional* newConditional();
Thread* newThread(Threadable* t);

} // namespace sdl1
} // namespace thread
} // namespace love
