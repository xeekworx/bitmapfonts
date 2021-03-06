#include "renderer_base.h"
#include <utf8.h>

#define XWF_DRAW_GUIDES 1

using namespace xeekworx::bitmapfonts;

namespace xeekworx {
    namespace bitmapfonts {
        struct bounds {
            int x, y, w, h;

            bounds(int x = 0, int y = 0, int w = 0, int h = 0)
                : x(x), y(y), w(w), h(h) {}

            bounds(const bounds& b, int inflateX = 0, int inflateY = 0)
                : bounds(b.x, b.y, b.w, b.h)
            {
                inflate(inflateX, inflateY);
            }

            bool contains(int sourceX, int sourceY) const
            {
                if (sourceX >= x && sourceX < x + w && sourceY >= y && sourceY <= y + h) return true;
                return false;
            }

            bool contains(const bounds& b) const {
                return contains(b.x, b.y) && contains(b.x + b.w, b.y + b.h);
            }

            bool has_intersection(const bounds& b) const
            {
                return 
                    contains(b.x, b.y) || contains(b.x + b.w, b.y + b.h) ||
                    b.contains(x, y) || b.contains(x+w, y+h);
            }

            void inflate(int offsetX, int offsetY)
            {
                x -= offsetX;
                y -= offsetY;
                w += offsetX * 2;
                h += offsetY * 2;
            }
        };
    }
}

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
    renderer_font new_font;

    if (font) {
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
    }

    for (auto image : m_font[static_cast<int>(style)].renderer_images)
        on_destroy_image(image);

    m_font[static_cast<int>(style)] = new_font;

    return old_font;
}

const xwf_font * renderer_base::get_font(font_style style) const
{
    return m_font[static_cast<int>(style)].source_font;
}

int renderer_base::get_line_xpos(const uint32_t* begin, const int length, int x, const int max_width) const
{
    // Nothign to do for left alignment:
    if (m_alignment == text_align::left) return x;

    // Find the end of the line:
    int end = length;
    for (int n = 0; n < length; ++n) 
        if (begin[n] == '\n') {
            end = n;
            break;
        }

    // Measure the line:
    int lineW = 0, lineH = 0;
    draw_internal(begin, end, 0, 0, lineW, lineH, 0, true);

    // Calculate the alignment based xpos:
    switch (m_alignment) {
    case text_align::center:
        if (m_textbox) x = x + ((max_width / 2) - (lineW / 2));
        else x = x - lineW / 2;
        break;
    case text_align::right:
        if (m_textbox) x = x + max_width - lineW;
        else x = x - lineW;
        break;
    }

    return x;
}

void renderer_base::draw_internal(
    const uint32_t* text, int length, 
    int _x, int _y, 
    int& width, int& height, 
    int padding, 
    const bool measure_only) const
{
    // ------------------------------------------------------------------------
    // SANITY CHECKS

    if (!m_font[static_cast<int>(font_style::normal)].source_font) return;
    if (length <= 0) return;        // TODO: Detect length of text
    if (measure_only) padding = 0;  // Ignore padding measuring text

    // ---

    const renderer_font& _font = m_font[static_cast<int>(font_style::normal)];
    const xwf_font * font = _font.source_font;
    const std::vector<uintptr_t>& images = _font.renderer_images;
    const bounds external_bounds(_x, _y, width, height);
    const bounds internal_bounds(external_bounds, -padding, -padding);

    // ------------------------------------------------------------------------
    // SMART TAB CALCULATION
    // Tab stops are based on the width of the internal bounds divided by
    // the pixel width of tabs. The lamda function is used when rendering.

    const int tab_start_x = internal_bounds.x;
    auto smart_tab = [](int start_x, int current_x, int tab_pixels) {
        return std::div(current_x - start_x, tab_pixels).rem * tab_pixels;
    };

    // ------------------------------------------------------------------------
    // DEBUG GUIDES RENDERING

#ifdef XWF_DRAW_GUIDES
    if (!measure_only) {
        // Center line:
        on_draw_line(
            external_bounds.x + external_bounds.w / 2,
            external_bounds.y,
            external_bounds.x + external_bounds.w / 2,
            external_bounds.y + external_bounds.h - 1,
            0x0000FFFF);
        // Padding (internal box):
        if (padding)
            on_draw_rect(
                internal_bounds.x,
                internal_bounds.y,
                internal_bounds.w,
                internal_bounds.h,
                0xFF0000FF, 1);
        // External bounds:
        on_draw_rect(
            external_bounds.x,
            external_bounds.y,
            external_bounds.w,
            external_bounds.h,
            0x000000FF, 1);
    }
#endif

    // ------------------------------------------------------------------------
    // STARTING HORIZONTAL POSITION OF TEXT
    // This is also recalculated later for every newline.

    int32_t startX = internal_bounds.x;
    int32_t startY = internal_bounds.y;
    if (!measure_only) {
        startX = get_line_xpos(text, length, startX, internal_bounds.w);
    }

    // ------------------------------------------------------------------------
    // RENDERING & MEASURING:
    // To get the actual width of rendered text we're going to have to simulate
    // drawing the text so that the advance width and bearing are calculated.

    int32_t measure_width = 0, measure_height = 0;
    uint32_t glyph_index = 0;

    for (int32_t i = 0, x = startX, y = font->font_size + startY, farthestX = 0, farthestY = 0; i < length; ++i) {
        // New-line handling:
        if (text[i] == (uint32_t) '\r') continue;
        else if (text[i] == (uint32_t) '\n') {
            // Text alignment handling:
            if (!measure_only) {
                startX = get_line_xpos(&text[i + 1], length - (i + 1), _x + padding, width - padding * 2);
            }
            x = startX;
            y += font->line_height;
            continue;
        }

        // Space & (Smart) Tab handling:
        if (text[i] == (uint32_t) ' ') {
            x += font->space_width;
            continue;
        }
        else if (text[i] == (uint32_t) '\t') {
            if(m_smart_tabs)
                x += smart_tab(tab_start_x, x, m_tab_size * font->space_width);
            else
                x += font->space_width * m_tab_size;
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
        // If textbox mode is on and clip is enabled, glyphs will not be 
        // rendered outside of the internal bounds.
        if (!measure_only) {
            if ((!m_clip || !m_textbox) || // Always render glyph without textmode or clipping
                (
                    m_clip && m_textbox && // Only render if glyph is inside the bounds
                    internal_bounds.contains(bounds(x, y, glyph.source_w, glyph.source_h)))
                ) {

                on_draw_image
                (
                    images[glyph.source_image],
                    glyph.source_x,
                    glyph.source_y,
                    glyph.source_w,
                    glyph.source_h,
                    x + glyph.bearing_left,
                    y - glyph.bearing_top,
                    glyph.source_w,
                    glyph.source_h,
                    m_foreground,
                    glyph.flipped ? rotation::right90degrees : rotation::none
                );
            }
        }

        // Move the pen:
        x += glyph.advance_x;
    }

    if (measure_only) {
        width = measure_width;
        height = measure_height;
    }
}

void renderer_base::measure(
    const wchar_t * text, int length,
    int * out_width, int * out_height)
{
    length = length < 0 ? (int)std::wcslen(text) : length;
    std::wstring in_text(text, text + length);

    // Convert it to utf-8
    std::string out_utf8;
    utf8::utf16to8(in_text.begin(), in_text.end(), std::back_inserter(out_utf8));

    // Convert it to utf-32
    std::string::iterator end_it = utf8::find_invalid(out_utf8.begin(), out_utf8.end());
    std::vector<uint32_t> out_utf32;
    utf8::utf8to32(out_utf8.begin(), end_it, std::back_inserter(out_utf32));

    int tmp_w = 0, tmp_h = 0;
    draw_internal(out_utf32.data(), (int)out_utf32.size(), 0, 0, tmp_w, tmp_h, 0, true);

    if (out_width) *out_width = tmp_w;
    if (out_height) *out_height = tmp_h;
}

void renderer_base::measure(
    const char * text, int length,
    int * out_width, int * out_height)
{
    length = length < 0 ? (int)std::strlen(text) : length;
    std::string in_text(text, text + length);

    // Convert it to utf-32
    std::string::iterator end_it = utf8::find_invalid(in_text.begin(), in_text.end());
    std::vector<uint32_t> out_utf32;
    utf8::utf8to32(in_text.begin(), end_it, std::back_inserter(out_utf32));

    int tmp_w = 0, tmp_h = 0;
    draw_internal(out_utf32.data(), (int)out_utf32.size(), 0, 0, tmp_w, tmp_h, 0, true);

    if (out_width) *out_width = tmp_w;
    if (out_height) *out_height = tmp_h;
}

void renderer_base::measure(
    const wchar_t * text,
    int * out_width, int * out_height)
{
    measure(text, -1, out_width, out_height);
}

void renderer_base::measure(
    const char * text,
    int * out_width, int * out_height)
{
    measure(text, -1, out_width, out_height);
}

void renderer_base::draw(const wchar_t * text, int length, int x, int y, int width, int height, int padding) const
{
    length = length < 0 ? (int)std::wcslen(text) : length;
    std::wstring in_text(text, text + length);

    // Convert to UTF-8:
    std::string out_utf8;
    utf8::utf16to8(in_text.begin(), in_text.end(), std::back_inserter(out_utf8));

    draw(out_utf8.c_str(), (int)out_utf8.size(), x, y, width, height, padding);
}

void renderer_base::draw(const char * text, int length, int x, int y, int width, int height, int padding) const
{
    length = length < 0 ? (int)std::strlen(text) : length;
    std::string in_text(text, text + length);

    // Replace invalid codepoints:
    std::string tmp;
    utf8::replace_invalid(in_text.begin(), in_text.end(), std::back_inserter(tmp), '?');
    in_text = tmp;

    // Convert to UTF-32:
    std::vector<uint32_t> out_utf32;
    utf8::utf8to32(in_text.begin(), in_text.end(), std::back_inserter(out_utf32));

    draw_internal(out_utf32.data(), (int)out_utf32.size(), x,  y, width, height, padding, false);
}
