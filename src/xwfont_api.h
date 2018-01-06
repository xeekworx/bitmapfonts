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

            struct generate_config {
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

            XWFONTAPI int generate(const generate_config * config);
            XWFONTAPI const char * get_error(void);

        }
    }
}