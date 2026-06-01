#include "./debug.hpp"
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
