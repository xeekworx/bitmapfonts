#include "renderer_base.h"
#include "image.h"

using namespace xeekworx::bitmapfonts;

renderer_base::renderer_base()
{

}

renderer_base::~renderer_base()
{
}

uint32_t renderer_base::set_foreground(uint32_t color)
{
    uint32_t old_color = m_foreground;
    m_foreground = color;
    return old_color;
}

uint32_t renderer_base::set_background(uint32_t color)
{
    uint32_t old_color = m_background;
    m_background = color;
    return old_color;
}

const xwf_font * renderer_base::set_font(const xwf_font * font, font_style style)
{
    const xwf_font * old_font = m_font[static_cast<int>(style)];
    m_font[static_cast<int>(style)] = font;
    return old_font;
}

const xwf_font * renderer_base::get_font(font_style style) const
{
    return m_font[static_cast<int>(style)];
}