#include "canvas.hpp"
#include "grrlib.h"
#include "sol/forward.hpp"
#include <malloc.h>
#include <ogc/cache.h>
#include <ogc/gu.h>
#include <ogc/gx.h>
#include <stdexcept>

namespace love {
namespace graphics {

static love_canvas_t *main_screen = nullptr;
static love_canvas_t *current_canvas;

void copyEfbToCurrentCanvas() {
  GX_SetTexCopySrc(0, 0, current_canvas->viewWidth, current_canvas->viewHeight);
  GX_SetTexCopyDst(current_canvas->buffer->w, current_canvas->buffer->h,
                   current_canvas->buffer->format, GX_FALSE);

  GX_CopyTex(current_canvas->buffer->data, GX_TRUE);
  GX_DrawDone();

  DCInvalidateRange(current_canvas->buffer->data, current_canvas->texSize);
  GX_InvalidateTexAll();
}

love_canvas_t *create_canvas(u16 w, u16 h) {

  if (w == 0 || h == 0) {
    w = 640;
    h = 480;
  }
  if (w > 640 || h > 480)
    throw std::runtime_error("Wii hardware limitation error : cant create a "
                             "canvas bigger than 640x480");

  love_canvas_t *c = (love_canvas_t *)malloc(sizeof(love_canvas_t));

  c->viewWidth = w;
  c->viewHeight = h;
  printf("Creating a canvas of %dx%d\n", w, h);

  c->buffer = GRRLIB_CreateEmptyTextureFmt(w, h, GX_TF_RGBA8);
  c->texSize =
      GX_GetTexBufferSize(c->buffer->w, c->buffer->h, GX_TF_RGBA8, GX_FALSE, 0);

  printf("Is the buffer 32b aligned ? : %d\n", ((int)c->buffer->data & 4) == 0);
  return c;
}

love_canvas_t *getCanvas() { return current_canvas; }

void set_canvas(love_canvas_t *canvas) {

  if(main_screen == nullptr)
  {
    main_screen = create_canvas(0, 0);
  }

  if(current_canvas == nullptr && canvas != nullptr)
  {
    current_canvas = main_screen;
    resolve_canvas();
    current_canvas = nullptr;
  }

  if (current_canvas != nullptr && current_canvas != canvas) {
    resolve_canvas();
  }

  Mtx44 proj;

  if (canvas != nullptr) {
    GX_SetViewport(0, 0, canvas->viewWidth, canvas->viewHeight, 0, 1);
    GX_SetScissor(0, 0, canvas->viewWidth, canvas->viewHeight);

    guOrtho(proj, 0, canvas->viewHeight, 0, canvas->viewWidth, 0, 300);
    GX_LoadProjectionMtx(proj, GX_ORTHOGRAPHIC);
 
  } else {
    GX_SetViewport(0, 0, 640, 480, 0, 1);
    GX_SetScissor(0, 0, 640, 480);

    guOrtho(proj, 0, 480, 0, 640, 0, 300);
    GX_LoadProjectionMtx(proj, GX_ORTHOGRAPHIC);
    GRRLIB_DrawImg(0, 0, main_screen->buffer, 0, 1, 1, 0xFFFFFFFF);
  }

  current_canvas = canvas;
}

void resolve_canvas() {
  if (current_canvas == nullptr || current_canvas->buffer == nullptr ||
      current_canvas->buffer->data == nullptr) {
    throw std::runtime_error("Resolving null canvas. Something went wrong in "
                             "the rendering pipeline");
  }
  GX_DrawDone();
  copyEfbToCurrentCanvas();
}

void free_canvas(love_canvas_t *canvas) {
  GRRLIB_FreeTexture(canvas->buffer);
  free(canvas);
}

void lua_setCanvas(sol::object c) {
  if (c == sol::nil || !c.is<love_canvas_t *>())
    set_canvas(nullptr);
  else
    set_canvas(c.as<love_canvas_t *>());
}

void lua_resolveCanvas(love_canvas_t *c) { resolve_canvas(); }

void lua_nativeDraw(love_canvas_t *c, float x, float y,
                    sol::variadic_args args) {
  if (c == nullptr || c->buffer == nullptr || c->buffer->data == nullptr)
    return;

  float degrees = 0.0f;
  float scaleX = 1.0f;
  float scaleY = 1.0f;
  u32 tintColor = 0xFFFFFFFF;

  if (args.size() > 0)
    degrees = args[0].as<float>();
  if (args.size() > 1)
    scaleX = args[1].as<float>();
  if (args.size() > 2)
    scaleY = args[2].as<float>();
  if (args.size() > 3)
    tintColor = args[3].as<u32>();

  GRRLIB_DrawImg(x, y, c->buffer, degrees, scaleX, scaleY, tintColor);
}

} // namespace graphics
} // namespace love
