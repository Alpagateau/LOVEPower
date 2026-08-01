#include "./debug.hpp"
#include "lua.h"
#include <cstdlib>
#include <stdio.h>

static FILE* debug_file;

void DebugInit()
{
  debug_file = fopen("LOVEPower.log", "w");
  if(debug_file == NULL)
    debug_file = stderr;
}

FILE* DebugGetFile()
{
  return debug_file;
}

int getCurrentLine(lua_State* L)
{
  lua_Debug ld = {};
  lua_getinfo(L, "lS", &ld);
  return ld.currentline;
}
