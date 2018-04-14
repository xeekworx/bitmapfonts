#pragma once

#include "../src/xwfont_api.h"

#ifdef XWFONT_RENDERER_SDL
#   include "../src/renderer_sdl.h"
#endif

#ifdef XWFONT_RENDERER_NANOVG
#   include "../src/renderer_nanovg.h"
#endif