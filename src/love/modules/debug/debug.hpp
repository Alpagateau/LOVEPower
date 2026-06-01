#ifndef LOVE_DEBUG_HPP
#define LOVE_DEBUG_HPP

#include <stdio.h>

#define DEBUG_LOG(s, ...) \
  fprintf(DebugGetFile(), "[LOVE : %s (%d)]\t:" s, __FILE__, __LINE__, ##__VA_ARGS__);

void DebugInit();
FILE* DebugGetFile();

#endif
