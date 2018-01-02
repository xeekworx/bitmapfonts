#pragma once
#include <stdint.h>

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
    extern "C" {

        inline uint32_t rgba_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            return ((r)+(g << 8) + (b << 16) + (a << 24));
        }

        inline uint32_t bgra_to_html(uint32_t color)
        {
            return ((((color >> 16) & 0xff))+(((color >> 8) & 0xff) << 8) + (((color) & 0xff) << 16) + (((color >> 24) & 0xff) << 24));
        }

        // Converts from an html ordered color to what xwf uses internally (BGRA)
        inline uint32_t html_color(uint32_t webhex) 
        {
            return (((webhex >> 16) & 0xff)) + (((webhex >> 8) & 0xff) << 8) + (((webhex) & 0xff) << 16) + (((webhex >> 24) & 0xff) << 24);
        }

        inline uint32_t html_color_ex(uint32_t webhex, uint8_t alpha)
        {
            return (((webhex >> 16) & 0xff)) + (((webhex >> 8) & 0xff) << 8) + (((webhex) & 0xff) << 16) + (alpha << 24);
        }

        XWFONTAPI int xwf_test(const char * font_path, long font_size, unsigned long start_char, unsigned long end_char, uint32_t foreground = 0xFF000000, uint32_t background = 0xFF0000FF);
        XWFONTAPI const char * xwf_geterror(void);

    }
}