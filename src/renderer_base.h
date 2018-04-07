#pragma once
#include <stdint.h>
#include <vector>
#include "color.h"
#include "xwfont_types.h"
#include <array>
#include <vector>

namespace xeekworx {
    namespace bitmapfonts {

        class renderer_base {
        private:
            struct renderer_font {
                const xwf_font * source_font = nullptr;
                std::vector<uintptr_t> renderer_images; // contexts for images converted to a renderer form (eg. opengl texture name)
            };

            std::array<renderer_font, 3> m_font;
            color m_foreground, m_background;
            uint32_t m_tab_size = 4;
            bool m_smart_tabs = true;

        public:
            renderer_base();
            virtual ~renderer_base();

            virtual color set_foreground(color color);
            virtual color set_background(color color);
            virtual const xwf_font * set_font(const xwf_font * font, font_style style = font_style::normal);

            color get_foreground() const { return m_foreground; }
            color get_background() const { return m_background; }
            const xwf_font * get_font(font_style style = font_style::normal) const;
            const std::array<renderer_font, 3> get_fonts() const { return m_font; }

            virtual bool setup_testenv(const int width, const int height, const bool hidden, char * error = nullptr, const size_t error_max = 0) = 0;
            virtual bool poll_testenv() = 0;
            virtual void close_testenv() = 0;
            virtual bool is_testenv_setup() = 0;

            void measure(const wchar_t * text, int length, int * out_width, int * out_height);
            void measure(const char * text, int length, int * out_width, int * out_height);
            void measure(const wchar_t * text, int * out_width, int * out_height);
            void measure(const char * text, int * out_width, int * out_height);

            void draw(const wchar_t * text, int length, int x, int y, int width, int height, int padding = 0) const;
            void draw(const char * text, int length, int x, int y, int width, int height, int padding = 0) const;

        private:
            void draw_internal(const uint32_t* text, int length, int x, int y, int& width, int& height, int padding = 0,  const bool measure_only = false) const;

        protected:
            enum class rotation { none, left90degrees, right90degrees };

            virtual uintptr_t on_create_image(const uint32_t * data, int32_t width, int32_t height, int32_t channels) = 0;
            virtual void on_destroy_image(uintptr_t image) = 0;
            virtual void on_draw_line(const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const color& clr) const = 0;
            virtual void on_draw_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const color& clr, const uint32_t thickness = 1) const = 0;
            virtual void on_fill_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const color& clr) const = 0;
            virtual void on_draw_image(
                uintptr_t source_img,
                const int32_t source_x, const int32_t source_y,
                const int32_t source_w, const int32_t source_h,
                const int32_t x, const int32_t y,
                const int32_t width, const int32_t height,
                const color& modulation,
                const rotation direction = rotation::none
            ) const = 0;
        };

    }
}