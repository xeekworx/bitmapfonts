#pragma once
#include <stdint.h>
#include <vector>
#include "renderer_base.h"

namespace xeekworx {
    namespace bitmapfonts {

        class renderer_sdl : public renderer_base {
        private:
            void * m_sdl_renderer = nullptr;

        public:
            renderer_sdl(void * sdl_renderer);
            virtual ~renderer_sdl();

            const xwf_font * set_font(const xwf_font * font, font_style style = font_style::normal) override;

            bool setup_testenv(const int width, const int height, const bool hidden, char * error, const size_t error_max) override;
            void close_testenv() override;
            bool is_testenv_setup() override;

        protected:
            uintptr_t on_create_image(const uint32_t * data, int32_t width, int32_t height, int32_t channels) override;
            void on_destroy_image(uintptr_t image) override;
            void on_draw_line(const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const color& clr) override;
            void on_draw_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const color& clr, const uint32_t thickness = 1) override;
            void on_fill_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const color& clr) override;
            void on_draw_image(
                uintptr_t source_img,
                const int32_t source_x, const int32_t source_y,
                const int32_t source_w, const int32_t source_h,
                const int32_t x, const int32_t y,
                const int32_t width, const int32_t height,
                const color& modulation = color::white,
                const rotation direction = rotation::none
            ) override;
        };

    }
}