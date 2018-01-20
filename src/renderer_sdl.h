#pragma once
#include <stdint.h>
#include <vector>
#include "renderer_base.h"

namespace xeekworx {
    namespace bitmapfonts {

        class renderer_sdl : public renderer_base {
        public:
            renderer_sdl();
            virtual ~renderer_sdl();

            uint32_t set_foreground(uint32_t color) override;
            uint32_t set_background(uint32_t color) override;
            const xwf_font * set_font(const xwf_font * font, font_style style = font_style::normal) override;

            bool setup_testenv(const int width, const int height, const bool hidden, char * error, const size_t error_max) override;
            void close_testenv() override;
            bool is_testenv_setup() override;
        };

    }
}