#pragma once
#include <stdint.h>

namespace xeekworx {
    namespace bitmapfonts {
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
                uint32_t font_size = 0;             // Original font size (em / pixels)
                uint32_t line_height = 0;           // Height of a line of text containing any glyph
                uint32_t start_codepoint = 0;       // Starting unicode codepoint
                uint32_t num_glyph_indices = 0;     // Number of glyph indices
                uint32_t * glyph_indices = nullptr; // Ordered in unicode codepoints
                xwf_glyph * glyphs = nullptr;       // Glyph array corresponding to a glyph index
                uint32_t num_glyphs = 0;
                xwf_image * images = nullptr;
                uint32_t num_images = 0;
            };

        }
    }
}
