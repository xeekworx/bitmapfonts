#pragma once
#include <stdint.h>
#include <memory>
#include <algorithm>
#include "xwfont_types.h"

#ifdef XWFONTLIBRARY_EXPORTS
#   define XWFONTAPI __declspec(dllexport)
#else
#   ifdef XWFONTLIBRARY_SHARED
#       define XWFONTAPI __declspec(dllimport)
#   else
#       define XWFONTAPI
#   endif
#endif

namespace xeekworx {
    namespace bitmapfonts {
        extern "C" {

            XWFONTAPI const char * get_error(void);
            XWFONTAPI xwf_font * generate_font(const xwf_generation_config * config);
            XWFONTAPI int delete_font(xwf_font * font);
            XWFONTAPI void set_sample_background(uint32_t background = 0xFFFFFF00);
            XWFONTAPI int render_sample_utf8(const xwf_font * font, const char * text, int length, int width, int height, int padding = 0);
            XWFONTAPI int render_sample_utf16(const xwf_font * font, const wchar_t * text, int length, int width, int height, int padding = 0);
            XWFONTAPI int measure_sample_utf8(const xwf_font * font, const char * text, int length, int * out_width, int * out_height);
            XWFONTAPI int measure_sample_utf16(const xwf_font * font, const wchar_t * text, int length, int * out_width, int * out_height);
        }
    }
}