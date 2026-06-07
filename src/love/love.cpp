#include <string>
#include <tuple>

#include <sol/sol.hpp>
extern "C" {
#include <lua.h>
#include <stdlib.h>
}

#include "love.hpp"

//#include "modules/graphics/classes/font.hpp"
//#include "modules/graphics/classes/texture.hpp"

//#include "modules/wiimote/classes/wiimoteController.hpp"

#include "modules/audio/audio.hpp"
#include "modules/data/data.hpp"
#include "modules/event/event.hpp"
#include "modules/filesystem/filesystem.hpp"
#include "modules/graphics/graphics.hpp"
#include "modules/math/math.hpp"
#include "modules/system/system.hpp"
#include "modules/timer/timer.hpp"
#include "modules/wiimote/wiimote.hpp"

#ifndef NO_LIBMII
#include "modules/mii/miimodule.hpp"
#endif
#include "modules/physics/box2d/wrap_Physics.h"
#include "modules/window/window.hpp"

//#include "common/Exception.h"

#include "arg_lua.h"
#include "boot_lua.h"
#include "callbacks_lua.h"
#include "jitsetup_lua.h"
#include "nogame_lua.h"

#include <grrlib.h>

#include <fstream>
#include <iostream>

//#include "dirent.h"

#define LOVE_VERSION_MAJOR 0
#define LOVE_VERSION_MINOR 1
#define LOVE_VERSION_REVISION 0
#define LOVE_VERSION_SUFFIX "UNKNOWN"
#define LOVE_VERSION_STRING                                                    \
  (std::to_string(LOVE_VERSION_MAJOR) + "." +                                  \
   std::to_string(LOVE_VERSION_MINOR) + "." +                                  \
   std::to_string(LOVE_VERSION_REVISION) + "-" + LOVE_VERSION_SUFFIX)

extern int luaopen_love_physics(lua_State *);

static const luaL_Reg modules[] = {
    {"love", luaopen_love},
    {"love.audio", luaopen_love_audio},
    {"love.data", luaopen_love_data},
    {"love.event", luaopen_love_event},
    {"love.filesystem", luaopen_love_filesystem},
    {"love.graphics", luaopen_love_graphics},
    {"love.math", luaopen_love_math},
#ifdef USE_LIBMII
    {"love.mii", luaopen_love_mii},
#endif
    {"love.physics", love::physics::box2d::luaopen_love_physics},
    {"love.system", luaopen_love_system},
    {"love.timer", luaopen_love_timer},
    {"love.wiimote", luaopen_love_wiimote},
    {"love.window", luaopen_love_window},
    {"love.nogame", luaopen_love_nogame},
    {"love.arg", luaopen_love_arg},
    {"love.callbacks", luaopen_love_callbacks},
    {"love.boot", luaopen_love_boot},
    {"love.jitsetup", luaopen_love_jitsetup},
    {NULL, NULL}};

static int love_preload(lua_State *L, lua_CFunction f,
                        const char *name) { // From Love2D
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "preload");
  lua_pushcfunction(L, f);
  lua_setfield(L, -2, name);
  lua_pop(L, 2);
  return 0;
}

int luaopen_love(lua_State *L) {
  sol::state_view luastate(L);

  printf("<== MODULE LOVE ==>\n");

  lua_getglobal(L, "love");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "love");
  }

  for (int i = 0; modules[i].name != nullptr; i++)
    love_preload(L, modules[i].func, modules[i].name);

  lua_getglobal(L, "require");
  lua_pushstring(L, "love.jitsetup");
  lua_call(L, 1, 1);

  static const char *MAIN_THREAD_KEY = "_love_mainthread";

  lua_getfield(L, LUA_REGISTRYINDEX, MAIN_THREAD_KEY);
  if (lua_isnoneornil(L, -1)) {
    lua_pop(L, 1);

    lua_pushthread(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, MAIN_THREAD_KEY);
  }

  luastate["love"]["_version"] = LOVE_VERSION_STRING;
  luastate["love"]["_version_major"] = LOVE_VERSION_MAJOR;
  luastate["love"]["_version_minor"] = LOVE_VERSION_MINOR;
  luastate["love"]["_version_revision"] = LOVE_VERSION_REVISION;
  luastate["love"]["_version_suffix"] = LOVE_VERSION_SUFFIX;
  luastate["love"]["getVersion"] = love::getVersion;
  luastate["love"]["getOS"] = []() { return love::_os; };
  luastate["love"]["getConsole"] = []() { return love::_console; };
  luastate["love"]["hasDeprecationOutput"] = love::hasDeprecationOutput;
  luastate["love"]["setDeprecationOutput"] = love::setDeprecationOutput;

  luastate["love"]["debugLog"] = [](const std::string &s) {
    printf("[LOVEPower] %s\n", s.c_str());
  };

  lua_getglobal(L, "love");
  return 1;
}

int luaopen_love_nogame(lua_State *L) {
  if (luaL_loadbuffer(L, (const char *)nogame_lua, nogame_lua_size,
                      "=[love \"nogame.lua\"]") == 0)
    lua_call(L, 0, 1);

  return 1;
}

int luaopen_love_arg(lua_State *L) {
  if (luaL_loadbuffer(L, (const char *)arg_lua, arg_lua_size,
                      "=[love \"arg.lua\"]") == 0)
    lua_call(L, 0, 1);

  return 1;
}

int luaopen_love_callbacks(lua_State *L) {
  if (luaL_loadbuffer(L, (const char *)callbacks_lua, callbacks_lua_size,
                      "=[love \"callbacks.lua\"]") == 0)
    lua_call(L, 0, 1);

  return 1;
}

int luaopen_love_boot(lua_State *L) {
  if (luaL_loadbuffer(L, (const char *)boot_lua, boot_lua_size,
                      "=[love \"boot.lua\"]") == 0)
    lua_call(L, 0, 1);

  return 1;
}

int luaopen_love_jitsetup(lua_State *L) {
  if (luaL_loadbuffer(L, (const char *)jitsetup_lua, jitsetup_lua_size,
                      "=[love \"jitsetup.lua\"]") == 0)
    lua_call(L, 0, 1);

  return 1;
}

namespace love {
void UNUSED();
void UNUSED(...) {};

void logError(const std::string &msg) {
  std::ofstream log("sd:/lovepower_cpp_error.log",
                    std::ios::app); // append mode
  if (log.is_open()) {
    log << msg << std::endl;
  }
  printf("[LOVE ERROR] %s\n", msg.c_str());
}

bool hasDeprecationOutput() { return _deprecationOutput; }

void setDeprecationOutput(bool deprecationOutput) {
  _deprecationOutput = deprecationOutput;
}

std::tuple<int, int, int, std::string> getVersion() { return _version; }

int initialize(int argc, char **argv) {
  // int retval = 1;
  sol::state luastate;

  try {
    luastate.open_libraries(sol::lib::base, sol::lib::package,
                            sol::lib::coroutine, sol::lib::string, sol::lib::os,
                            sol::lib::math, sol::lib::table, sol::lib::debug,
                            sol::lib::bit32, sol::lib::io, sol::lib::utf8
#ifdef USE_LUAJIT
                            ,
                            sol::lib::ffi, sol::lib::jit
#endif
    );

    lua_State *L = luastate.lua_state();

    love_preload(L, luaopen_love_jitsetup, "love.jitsetup");
    lua_getglobal(L, "require");
    lua_pushstring(L, "love.jitsetup");
    lua_call(L, 1, 0);
    luastate.open_libraries(sol::lib::base, sol::lib::package,
                            sol::lib::coroutine, sol::lib::string, sol::lib::os,
                            sol::lib::math, sol::lib::table, sol::lib::debug,
                            sol::lib::bit32, sol::lib::io, sol::lib::utf8,
                            sol::lib::ffi, sol::lib::jit);

    love_preload(L, luaopen_love, "love");
    lua_getglobal(L, "require");
    lua_pushstring(L, "love");
    lua_call(L, 1, 1);

    lua_pop(L, 1);

    love::event::__init(luastate);
    love::audio::__init(luastate);
    love::graphics::__init(luastate);
    love::filesystem::__init(luastate, argc, argv);
    love::timer::__init(luastate);
    love::wiimote::__init(luastate);
#ifdef USE_LIBMII
    love::mii::__init(luastate);
#endif

    luastate["arg"] = luastate.create_table();
    for (int i = 0; i < argc; i++) {
      luastate["arg"][i + 1] = argv[i];
    }

    lua_pop(L, 1);

    lua_getglobal(L, "require");
    lua_pushstring(L, "love.boot");
    lua_call(L, 1, 1);

    // retval = 0;
    int done = 0;

    lua_close(L);
    return done;
  } catch (const std::exception &e) {
    logError(std::string("Exception during initialization: ") + e.what());
    return 0;
  }
}

int exit() {
  GRRLIB_Exit();
  std::exit(0);

  return 0;
}
} // namespace love
