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

        for (auto image : m_font[static_cast<int>(style)].renderer_images)
            on_destroy_image(image);

        m_font[static_cast<int>(style)] = new_font;
    }

    return old_font;
}

const xwf_font * renderer_base::get_font(font_style style) const
{
    return m_font[static_cast<int>(style)].source_font;
}

void renderer_base::draw_text_internal(const uint32_t* text, int length, int x, int y, int& width, int& height, int padding, const bool measure_only) const
{
    // Sanity checks...
    if (!m_font[static_cast<int>(font_style::normal)].source_font) return;
    if (length <= 0) return;        // TODO: Detect length of text
    if (measure_only) padding = 0;  // Ignore padding measuring text

    const renderer_font& _font = m_font[static_cast<int>(font_style::normal)];
    const xwf_font * font = _font.source_font;
    const std::vector<uintptr_t>& images = _font.renderer_images;

    // SMART TAB CALCULATION:
    const int tab_start_x = x;
    auto smart_tab = [](int start_x, int current_x, int tab_pixels) {
        return std::div(current_x - start_x, tab_pixels).rem * tab_pixels;
    };

    // RENDERING & MEASUREMENT:
    // To get the actual width of rendered text we're going to have to simulate
    // drawing the text so that the advance width and bearing are calculated.

    int32_t startX = padding, startY = padding;
    int32_t measure_width = 0, measure_height = 0;
    uint32_t glyph_index = 0;

    for (int32_t i = 0, x = startX, y = font->font_size + startY, farthestX = 0, farthestY = 0; i < length; ++i) {
        // New-line handling:
        if (text[i] == (uint32_t) '\r') continue;
        else if (text[i] == (uint32_t) '\n') {
            x = startX;
            y += font->line_height;
            continue;
        }

        // Space & (Smart) Tab handling:
        if (text[i] == (uint32_t) ' ') {
            x += font->bbox_width;
            continue;
        }
        else if (text[i] == (uint32_t) '\t') {
            if(m_smart_tabs)
                x += smart_tab(tab_start_x, x, m_tab_size * font->bbox_width);
            else
                x += font->bbox_width * m_tab_size;
            continue;
        }

        // Get the glyph index from the codepoint, if there's no glyph to
        // get then try to use the ? (question mark) instead.
        if (text[i] < font->start_codepoint ||
            text[i] > font->start_codepoint + font->num_glyph_indices)
        {
            if (0x0000003f >= font->start_codepoint && 0x0000003f < font->start_codepoint + font->num_glyph_indices)
                glyph_index = font->glyph_indices[0x0000003f - font->start_codepoint];
            else continue;
        }
        else glyph_index = font->glyph_indices[text[i] - font->start_codepoint];

        if (glyph_index >= font->num_glyphs)
            continue; // TODO: Warn about something wrong with the font

        const xwf_glyph& glyph = font->glyphs[glyph_index];

        // Text Measurement farthest extents:
        if (measure_only) {
            farthestX = (x + glyph.bearing_left) + (!glyph.flipped ? glyph.source_w : glyph.source_h);
            farthestY = (y - glyph.bearing_top) + (!glyph.flipped ? glyph.source_h : glyph.source_w);
            if (farthestX > measure_width) measure_width = farthestX;
            if (farthestY > measure_height) measure_height = farthestY;
        }

        // Actual rendering here, if not just measuring:
        if (!measure_only) {
            on_draw_image
            (
                images[glyph.source_image],
                glyph.source_x,
                glyph.source_y,
                glyph.source_w,
                glyph.source_h,
                x + glyph.bearing_left,
                y + glyph.bearing_top,
                glyph.source_w,
                glyph.source_h,
                m_foreground,
                glyph.flipped ? rotation::right90degrees : rotation::none
            );
        }

        // Move the pen:
        x += glyph.advance_x;
    }

    if (measure_only) {
        width = measure_width;
        height = measure_height;
    }
}