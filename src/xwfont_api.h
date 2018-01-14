#pragma once
#include <stdint.h>
#include <memory>
#include <algorithm>

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
    namespace bitmap_fonts {
        extern "C" {

            struct xwf_generation_config {
                const char * font_path = nullptr;
                uint32_t font_size = 14;
                uint32_t begin_char = 0;
                uint32_t end_char = 255;
                uint32_t page_size = 768;
                uint32_t foreground = 0xFFFFFFFF;
                uint32_t background = 0x000000FF;
                uint32_t border = 0x00FF42DF;
                int32_t border_thickness = 1;
                int32_t padding = 1;
            };

            struct xwf_image {
                uint32_t * data = nullptr;
                int32_t width = 0, height = 0, channels = 0;
            };

            struct xwf_glyph {
                uint32_t character = 0;
                int32_t source_image = -1;
                int32_t source_x = 0, source_y = 0, source_w = 0, source_h = 0;
                int32_t advance_x = 0, advance_y = 0;
                int32_t bearing_left = 0, bearing_top = 0;
                bool flipped = false;
            };

            struct xwf_font {
                const char * name = nullptr;
                uint32_t font_size = 0; // original font size (em / pixels)
                uint32_t start_glyph_index = 0; // starting unicode code point
                uint32_t num_glyph_indexes = 0; // number of glyph indexes
                uint32_t * glyph_indexes = nullptr; // ordered in unicode code points
                xwf_glyph * glyphs = nullptr; // glyph array corresponding to a glyph index
                uint32_t num_glyphs = 0;
                xwf_image * images = nullptr;
                uint32_t num_images = 0;
            };

            XWFONTAPI const char * get_error(void);
            XWFONTAPI xwf_font * generate_font(const xwf_generation_config * config);
            XWFONTAPI int delete_font(xwf_font * font);
            XWFONTAPI void set_sample_background(uint32_t background = 0xFFFFFF00);
            XWFONTAPI int render_sample_utf8(const xwf_font * font, const char * text, int length, int width, int height);
            XWFONTAPI int render_sample_utf16(const xwf_font * font, const wchar_t * text, int length, int width, int height);
            XWFONTAPI int measure_sample_utf8(const xwf_font * font, const char * text, int length, int * out_width, int * out_height);
            XWFONTAPI int measure_sample_utf16(const xwf_font * font, const wchar_t * text, int length, int * out_width, int * out_height);
        }
    }
}