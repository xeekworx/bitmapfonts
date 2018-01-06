#include "image.h"
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT // Be sure to remove _CRT_SECURE_NO_WARNINGS from the project's properties when nothings merges fixes into stb
#include <stb_image_write.h>

using namespace xeekworx::bitmap_fonts;

image::image(const uint32_t width, const uint32_t height, const uint32_t background) : m_width(width), m_height(height) {
    if (width * height) {
        m_data = new uint32_t[width * height];
        std::fill_n(m_data, width * height, background);
    }
}

image::image(const image& source) : image(source.m_width, source.m_height) {
    if (source.m_width * source.m_height) memcpy(m_data, source.m_data, size_in_bytes());
}

image::~image() {
    if (m_data) delete[] m_data;
}

void image::clear(const uint32_t background) {
    if (!empty()) std::fill_n(m_data, size(), background);
}

// Takes html colors and spits out BGRA
uint32_t image::blend_colors(const uint32_t foreground, const uint32_t background)
{
    const pixel_rgba dest(background);
    const pixel_rgba source(foreground);

    if (dest.a == 0) return foreground;

    pixel_rgba result;

    result.a = source.a + dest.a * (255 - source.a) / 255;
    result.r = (source.r * source.a + dest.r * dest.a * (255 - source.a) / 255) / result.a;
    result.g = (source.g * source.a + dest.g * dest.a * (255 - source.a) / 255) / result.a;
    result.b = (source.b * source.a + dest.b * dest.a * (255 - source.a) / 255) / result.a;

    return result;
}

void image::draw_hline(const int32_t x1, const int32_t x2, const int32_t y, const uint32_t html_color)
{
    if (y >= 0 && y < m_height)
        for (int32_t x = x1; x < x2; ++x)
            if (x >= 0 && x < m_width)
                to_point(x, y, blend_colors(html_color, from_point(x, y)));
}

void image::draw_vline(const int32_t x, const int32_t y1, const int32_t y2, const uint32_t html_color)
{
    if (x >= 0 && x < m_width)
        for (int32_t y = y1; y < y2; ++y)
            if (y >= 0 && y < m_height)
                to_point(x, y, blend_colors(html_color, from_point(x, y)));
}

void image::draw_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const uint32_t thickness, const uint32_t html_color)
{
    // left
    for (uint32_t t = 0; t < thickness; ++t)
        draw_vline(x + t, y, y + h, html_color);

    // top
    for (uint32_t t = 0; t < thickness; ++t)
        draw_hline(x + thickness, x + w, y + t, html_color);

    // right
    for (uint32_t t = 0; t < thickness; ++t)
        draw_vline((x + w - 1) - t, y + thickness, y + h, html_color);

    // bottom
    for (uint32_t t = 0; t < thickness; ++t)
        draw_hline(x + thickness, x + w - thickness, (y + h - 1) - t, html_color);
}

void image::draw_bitmap(const FT_Bitmap* ftbitmap, const int32_t x, const int32_t y, const uint32_t foreground)
{
    int32_t  i, j, p, q;
    int32_t  x_max = x + ftbitmap->width;
    int32_t  y_max = y + ftbitmap->rows;

    for (i = x, p = 0; i < x_max; i++, p++)
    {
        for (j = y, q = 0; j < y_max; j++, q++)
        {
            if (i < 0 || j < 0 || i >= m_width || j >= m_height) continue;

            to_point(i, j,
                blend_colors(
                    pixel_rgba(foreground, ftbitmap->buffer[q * ftbitmap->width + p]), // Target Color
                    from_point(i, j) // Background Color
                )
            );
        }
    }
}

bool image::save(const std::string& file) const
{
    std::vector<uint32_t> converted(m_data, m_data + size());

    // PNG expects a BGRA format (ARGB in reverse)
    for (uint32_t& p : converted) p = pixel_bgra(pixel_rgba(p));

    if (stbi_write_png(
        file.c_str(),
        m_width, m_height,
        channels,
        converted.data(),
        0))
        return true;
    else return false;
}