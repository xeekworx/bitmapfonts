#pragma once

#include "../src/xwfont_api.h"

#ifdef XWFONT_RENDERER_SDL
#   include "../src/renderer_sdl.h"
namespace xeekworx {
    namespace bitmapfonts {
        using renderer = renderer_sdl;
} }
#endif