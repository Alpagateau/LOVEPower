#include <sol/sol.hpp>
#include <vector>
#include <queue>
#include <tuple>
#include <cstdlib>
#include <grrlib.h>
#include <wiiuse/wpad.h>
#include <cstdlib>
#include "event.hpp"
#include <ogc/system.h>
#include "../wiimote/wiimote.hpp"
#include "sol/forward.hpp"
#include "sol/object.hpp"
#include "sol/variadic_results.hpp"
extern "C" {
    #include <lua.h>
} 
#include <malloc.h>
#include <ogcsys.h>

namespace love {
    namespace event {
        std::queue<event_t> events;
        //std::queue<event_t>::iterator currentEvent = events.end();

        static volatile bool requestQuit = false;
        static volatile bool requestReset = false;

        static float lastMouseX = 0.0f;
        static float lastMouseY = 0.0f;
        static bool lastMouseValid = false;
        static bool lastAHeld = false;
        static bool lastBHeld = false;

        static void onReset(u32 reset, void* usrdata) {
            requestReset = true;
        }

        static void onPower(void) {
            requestQuit = true;
        }

        static void onWpadPower(int chan) {
            requestQuit = true;
        }

        static bool checkLowMemory() {
            // guh???
            u32 mem1Free = (u32)((uintptr_t)SYS_GetArena1Hi() - (uintptr_t)SYS_GetArena1Lo());
            u32 mem2Free = (u32)((uintptr_t)SYS_GetArena2Hi() - (uintptr_t)SYS_GetArena2Lo());

            struct mallinfo mi = mallinfo();

            const u32 MEM1_THRESHOLD = 512 * 1024;
            const u32 MEM2_THRESHOLD = 2 * 1024 * 1024;
            const u32 MALLOC_THRESHOLD = 512 * 1024;

            return (mem1Free < MEM1_THRESHOLD) || 
                (mem2Free < MEM2_THRESHOLD) || 
                (mi.fordblks < (int)MALLOC_THRESHOLD);
        }

        void __init(sol::state & luastate) {
            SYS_SetResetCallback(onReset);
            SYS_SetPowerCallback(onPower);
            WPAD_SetPowerButtonCallback(onWpadPower);
        }

        void __pushEvent(sol::this_state lua, const char *eventName,
                         sol::object a = sol::lua_nil, sol::object b = sol::lua_nil,
                         sol::object c = sol::lua_nil, sol::object d = sol::lua_nil,
                         sol::object e = sol::lua_nil, sol::object f = sol::lua_nil) {
            printf("[EVENTS] Pushing event [%s]\n", eventName);
            push(sol::make_object(lua, eventName), a, b, c, d, e, f, lua);
        }

        void pump(sol::this_state lua) {
            //currentEvent = events.begin();
            
            love::wiimote::update();

            if (requestQuit || requestReset) {
                __pushEvent(lua, "quit");
                requestQuit = false;
                requestReset = false;
            }            

            if (checkLowMemory()) {
                __pushEvent(lua, "lowmemory");
            }

            love::wiimote::WiimoteController* remote = love::wiimote::getWiimote(1);
            if(remote && remote->isConnected() && remote->data)
            {
              bool mouse_valid = remote->data->ir.valid;
              float x = remote->getSmoothX();
              float y = remote->getSmoothY();

              if(mouse_valid)
              {
                float dx = 0;
                float dy = 0;
                if(lastMouseValid)
                {
                  dx = x - lastMouseX;
                  dy = y - lastMouseY;
                }

                if(!lastMouseValid || dx != 0 || dy != 0)
                {
                  __pushEvent(lua, "mousemoved", 
                      sol::make_object(lua, x),
                      sol::make_object(lua, y),
                      sol::make_object(lua, dx),
                      sol::make_object(lua, dy),
                      sol::make_object(lua, false)
                      );
                }

                lastMouseX = x;
                lastMouseY = y;
              }
              lastMouseValid = mouse_valid;
              float clickx = mouse_valid ? x : lastMouseX;
              float clicky = mouse_valid ? y : lastMouseY;

              bool a_held = remote->checkButton("a");
              bool b_held = remote->checkButton("b");

              if(!lastAHeld && a_held)
              {
                __pushEvent(lua, "mousepressed",
                    sol::make_object(lua, clickx),
                    sol::make_object(lua, clicky),
                    sol::make_object(lua, 1),
                    sol::make_object(lua, false),
                    sol::make_object(lua, 1));
              }
              if(lastAHeld && !a_held)
              {
                __pushEvent(lua, "mousereleased",
                    sol::make_object(lua, clickx),
                    sol::make_object(lua, clicky),
                    sol::make_object(lua, 1),
                    sol::make_object(lua, false),
                    sol::make_object(lua, 1));
              }
              if(!lastBHeld && b_held)
              {
                __pushEvent(lua, "mousepressed",
                    sol::make_object(lua, clickx),
                    sol::make_object(lua, clicky),
                    sol::make_object(lua, 2),
                    sol::make_object(lua, false),
                    sol::make_object(lua, 1));
              }
              if(lastBHeld && !b_held)
              {
                __pushEvent(lua, "mousereleased",
                    sol::make_object(lua, clickx),
                    sol::make_object(lua, clicky),
                    sol::make_object(lua, 2),
                    sol::make_object(lua, false),
                    sol::make_object(lua, 1));
              }

              lastAHeld = a_held;
              lastBHeld = b_held;
            }
        }

        sol::object poll(sol::this_state lua) {

          return sol::make_object(lua, []() {
            sol::variadic_results vr;
            if (events.empty()) {
                //events.clear();
                //currentEvent = events.end();
                return vr;
                //return std::make_tuple(sol::lua_nil, sol::lua_nil, sol::lua_nil,
                //                      sol::lua_nil, sol::lua_nil, sol::lua_nil, sol::lua_nil);
            } else {
                event_t e = events.front();
                printf("[EVENTS POLL] Polling");
                print_event_name(e);
                printf("\n");
                events.pop();

                vr.push_back(std::get<0>(e));
                vr.push_back(std::get<1>(e));
                vr.push_back(std::get<2>(e));
                vr.push_back(std::get<3>(e));
                vr.push_back(std::get<4>(e));
                vr.push_back(std::get<5>(e));
                vr.push_back(std::get<6>(e));

                return vr;
            }
            }
          );
        }

        void push(sol::object name, sol::object a, sol::object b,
                  sol::object c, sol::object d, sol::object e,
                  sol::object f, sol::this_state s) {
            events.push(std::make_tuple(name, a, b, c, d, e, f));
        }


        void quit(sol::this_state lua) {
            __pushEvent(lua, "quit");

            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
            std::exit(0);
        }

        void print_event_name(event_t& e)
        {
          sol::object c = std::get<0>(e);
          printf("<%s>", c.as<std::string>().c_str());
        }
    }
}

int luaopen_love_event(lua_State *L) {

    printf("<== MODULE LOVE EVENT ==>\n");
    sol::state_view luastate(L);

    while(!love::event::events.empty())
      love::event::events.pop();

    luastate["love"]["event"] = luastate.create_table_with(
        "pump", love::event::pump,
        "poll", love::event::poll,
        "push", love::event::push,
        "quit", love::event::quit
    );

    return 1;
}
