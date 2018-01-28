#include "renderer_base.h"
#include "image.h"

using namespace xeekworx::bitmapfonts;

renderer_base::renderer_base()
    : m_foreground(color::black), m_background(color::transparent)
{

}

renderer_base::~renderer_base()
{

}

color renderer_base::set_foreground(color color)
{
    xeekworx::bitmapfonts::color old_color(m_foreground);
    m_foreground = color;
    return old_color;
}

color renderer_base::set_background(color color)
{
    xeekworx::bitmapfonts::color old_color(m_background);
    m_background = color;
    return old_color;
}

const xwf_font * renderer_base::set_font(const xwf_font * font, font_style style)
{
    const xwf_font * old_font = m_font[static_cast<int>(style)].source_font;

    if (font) {
        renderer_font new_font;
        new_font.source_font = font;
        new_font.renderer_images.resize(font->num_images);

        for (uint32_t i = 0; i < font->num_images; ++i) {
            new_font.renderer_images[i] =
                on_create_image(
                    font->images[i].data,
                    font->images[i].width,
                    font->images[i].height,
                    font->images[i].channels
                );

            if (new_font.renderer_images[i] == 0) {
                return nullptr;
            }
        }

        m_font[static_cast<int>(style)] = new_font;
    }

    for (auto image : m_font[static_cast<int>(style)].renderer_images)
        on_destroy_image(image);

    return old_font;
}

const xwf_font * renderer_base::get_font(font_style style) const
{
    return m_font[static_cast<int>(style)].source_font;
}