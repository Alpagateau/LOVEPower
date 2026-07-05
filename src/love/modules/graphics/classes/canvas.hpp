#ifndef LOVE_CANVAS_HPP
#define LOVE_CANVAS_HPP

#include "sol/sol.hpp"
#include "grrlib.h"
#include <gctypes.h>

namespace love{
namespace graphics{

typedef struct {
  GRRLIB_texImg* buffer;
  u16 viewWidth;
  u16 viewHeight;
} love_canvas_t;

love_canvas_t* create_canvas(u16 w, u16 h);
void set_canvas(love_canvas_t* canvas);
void resolve_canvas(love_canvas_t* canvas);
love_canvas_t* getCanvas();
void free_canvas(love_canvas_t* canvas);

void lua_setCanvas(sol::object c);
void lua_resolveCanvas(love_canvas_t* c);
void lua_nativeDraw(love_canvas_t* c, float x, float y, sol::variadic_args args);

}}
#endif
