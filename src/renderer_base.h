#pragma once
#include <stdint.h>
#include <vector>
#include "xwfont_types.h"

namespace xeekworx {
    namespace bitmapfonts {

        class renderer_base {
        private:
            const xwf_font * m_font[3] = { nullptr }; // normal, bold, italic
            uint32_t m_foreground = 0xFFFFFFFF;
            uint32_t m_background = 0xFFFFFF00;

        public:
            renderer_base();
            virtual ~renderer_base();

            virtual uint32_t set_foreground(uint32_t color);
            virtual uint32_t set_background(uint32_t color);
            virtual const xwf_font * set_font(const xwf_font * font, font_style style = font_style::normal);

            uint32_t get_foreground() const { return m_foreground; }
            uint32_t get_background() const { return m_background; }
            const xwf_font * get_font(font_style style = font_style::normal) const;

            virtual bool setup_testenv(const int width, const int height, const bool hidden, char * error = nullptr, const size_t error_max = 0) = 0;
            virtual void close_testenv() = 0;
            virtual bool is_testenv_setup() = 0;
        };

    }
}