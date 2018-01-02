#pragma once
#include <stdint.h>
#include <memory>

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
                long font_size = 14;
                unsigned long begin_char = 0;
                unsigned long end_char = 255;
                unsigned int page_size = 768;

                uint32_t foreground = 0xFF000000;
                uint32_t background = 0x00FFFFFF;
                uint32_t border = 0xDF00FF42;
                unsigned int border_thickness = 1;

                unsigned int padding = 1;
            };

            class image {
            public:
                static constexpr uint32_t transparent = 0xFFFFFF00;

                uint32_t * pixels = nullptr;
                static constexpr uint32_t channels = 4;
                int32_t w = 0, h = 0; // signed, because x / y are

                image(uint32_t width, uint32_t height, uint32_t background = transparent) : w(width), h(height) {
                    if (width * height) {
                        pixels = new uint32_t[width * height];
                        for (int32_t i = 0; i < size(); ++i) pixels[i] = background;
                    }
                }

                image(const image& source) : image(source.w, source.h) {
                    if (source.w * source.h) memcpy(pixels, source.pixels, size_in_bytes());
                }

                ~image() {
                    if (pixels) delete[] pixels;
                }

                void clear(uint32_t background = transparent) { 
                    if (!empty()) for (int32_t i = 0; i < size(); ++i) pixels[i] = background;
                }
                bool empty() const { return size() > 0 && pixels; }
                size_t size_in_bytes() const  { return (w * channels * h); }
                size_t size() const { return w * h; }

                uint32_t& operator[](const int i) { return pixels[i]; }
                const uint32_t& operator[](const int i) const { return pixels[i]; }

                uint32_t from_point(uint32_t x, uint32_t y) { return pixels[x + w * y]; }
                void to_point(uint32_t x, uint32_t y, uint32_t value) { pixels[x + w * y] = value; }
            };

            XWFONTAPI int generate(const generate_config * config);
            XWFONTAPI const char * get_error(void);

        }
    }
}