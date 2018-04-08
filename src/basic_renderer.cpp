#include "basic_renderer.h"
#include <vector>
#include <fstream>

#define _USE_MATH_DEFINES
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT // Be sure to remove _CRT_SECURE_NO_WARNINGS from the project's properties when nothings merges fixes into stb
#include <stb_image_write.h>

using namespace xeekworx::bitmapfonts;

basic_renderer::basic_renderer(const uint32_t width, const uint32_t height, const uint32_t background) 
    : m_width(width), m_height(height), m_external_data(false)
{
    if (width * height) {
        m_data = new uint32_t[width * height];
        std::fill_n(m_data, width * height, background);
    }
}

basic_renderer::basic_renderer(const uint32_t width, const uint32_t height)
    : m_width(width), m_height(height), m_external_data(false)
{
    if (width * height) m_data = new uint32_t[width * height];
}

basic_renderer::basic_renderer(const basic_renderer& source) 
    : basic_renderer(source.m_width, source.m_height)
{
    if (source.m_width * source.m_height) 
        std::memcpy(m_data, source.m_data, size_in_bytes());
}

basic_renderer::basic_renderer(uint32_t * source_pixels, const uint32_t width, const uint32_t height, const bool copy_convert) 
    : m_width(width), m_height(height), m_external_data(!copy_convert)
{
    if (copy_convert) {
        if (width * height) {
            m_data = new uint32_t[width * height];
            for (uint32_t dest_cx = 0; dest_cx < width * height; ++dest_cx)
                m_data[dest_cx] = pixel_rgba(pixel_abgr(source_pixels[dest_cx]));
        }
    }
    else m_data = source_pixels;
}

basic_renderer::basic_renderer(const FT_Bitmap* ftbitmap, const uint32_t foreground, const uint32_t background)
    : basic_renderer(ftbitmap->width, ftbitmap->rows)
{
    for (int32_t i = 0; i < size(); ++i)
        m_data[i] = blend_colors(
            pixel_rgba(foreground, ftbitmap->buffer[i]),
            background
        );
}

basic_renderer::~basic_renderer()
{
    if (m_data && !m_external_data) {
        delete[] m_data;
        m_data = nullptr;
    }
}

void basic_renderer::clear(const uint32_t background)
{
    if (!empty()) std::fill_n(m_data, size(), background);
}

// Takes html colors and spits out BGRA
uint32_t basic_renderer::blend_colors(const uint32_t foreground, const uint32_t background)
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

void basic_renderer::draw_hline(const int32_t x1, const int32_t x2, const int32_t y, const uint32_t color)
{
    if (y >= 0 && y < m_height)
        for (int32_t x = x1; x < x2; ++x)
            if (x >= 0 && x < m_width)
                to_point(x, y, blend_colors(color, from_point(x, y)));
}

void basic_renderer::draw_vline(const int32_t x, const int32_t y1, const int32_t y2, const uint32_t color)
{
    if (x >= 0 && x < m_width)
        for (int32_t y = y1; y < y2; ++y)
            if (y >= 0 && y < m_height)
                to_point(x, y, blend_colors(color, from_point(x, y)));
}

void basic_renderer::draw_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const uint32_t thickness, const uint32_t color)
{
    // left
    for (uint32_t t = 0; t < thickness; ++t)
        draw_vline(x + t, y, y + h, color);

    // top
    for (uint32_t t = 0; t < thickness; ++t)
        draw_hline(x + thickness, x + w, y + t, color);

    // right
    for (uint32_t t = 0; t < thickness; ++t)
        draw_vline((x + w - 1) - t, y + thickness, y + h, color);

    // bottom
    for (uint32_t t = 0; t < thickness; ++t)
        draw_hline(x + thickness, x + w - thickness, (y + h - 1) - t, color);
}

void basic_renderer::draw_bitmap(const basic_renderer& source_img, const int32_t source_x, const int32_t source_y, const int32_t source_w, const int32_t source_h, const int32_t x, const int32_t y)
{
    int32_t  dest_cx, dest_cy, source_cx, source_cy;
    int32_t  x_max = x + source_w;
    int32_t  y_max = y + source_h;

    for (dest_cx = x, source_cx = source_x; dest_cx < x_max; dest_cx++, source_cx++)
    {
        for (dest_cy = y, source_cy = source_y; dest_cy < y_max; dest_cy++, source_cy++)
        {
            if (dest_cx < 0 || dest_cy < 0 || dest_cx >= m_width || dest_cy >= m_height) continue;

            to_point(dest_cx, dest_cy,
                blend_colors(
                    source_img.from_point(source_cx, source_cy),
                    from_point(dest_cx, dest_cy) // Background Color
                )
            );
        }
    }
}

void basic_renderer::draw_bitmap(const FT_Bitmap* ftbitmap, const int32_t x, const int32_t y, const uint32_t foreground)
{
    draw_bitmap(
        basic_renderer(ftbitmap, foreground, basic_renderer::transparent),
        0, 0, ftbitmap->width, ftbitmap->rows,
        x, y
    );
}

void basic_renderer::draw_bitmap_rotated(const basic_renderer& source_img, const int32_t source_x, const int32_t source_y, const int32_t source_w, const int32_t source_h, const int32_t x, const int32_t y, rotation direction)
{
    int32_t dest_cx, dest_cy, source_cx, source_cy;
    const int32_t dest_x_max = x + source_h;
    const int32_t dest_y_max = y + source_w;

    if (direction == rotation::right90degrees) {
        for (dest_cy = y, source_cx = source_x; dest_cy < dest_y_max; dest_cy++, source_cx++)
        {
            for (dest_cx = x, source_cy = source_y + (source_h - 1); dest_cx < dest_x_max; dest_cx++, source_cy--)
            {
                if (dest_cx < 0 || dest_cy < 0 || dest_cx > m_width || dest_cy > m_height) continue;
                if (source_cx < 0 || source_cy < 0 || source_cx > source_img.width() || source_cy > source_img.height()) continue;

                to_point(dest_cx, dest_cy,
                    blend_colors(
                        source_img.from_point(source_cx, source_cy),
                        from_point(dest_cx, dest_cy) // Background Color
                    )
                );
            }
        }
    }
    else {
        for (dest_cy = y, source_cx = source_x + (source_w - 1); dest_cy < dest_y_max; dest_cy++, source_cx--)
        {
            for (dest_cx = x, source_cy = source_y; dest_cx < dest_x_max; dest_cx++, source_cy++)
            {
                if (dest_cx < 0 || dest_cy < 0 || dest_cx > m_width || dest_cy > m_height) continue;
                if (source_cx < 0 || source_cy < 0 || source_cx > source_img.width() || source_cy > source_img.height()) continue;

                to_point(dest_cx, dest_cy,
                    blend_colors(
                        source_img.from_point(source_cx, source_cy),
                        from_point(dest_cx, dest_cy) // Background Color
                    )
                );
            }
        }
    }
}

void basic_renderer::draw_bitmap_rotated(const FT_Bitmap* ftbitmap, const int32_t x, const int32_t y, const uint32_t foreground, rotation direction)
{
    draw_bitmap_rotated(
        basic_renderer(ftbitmap, foreground, basic_renderer::transparent), 
        0, 0, ftbitmap->width, ftbitmap->rows, 
        x, y, 
        direction
    );
}

bool basic_renderer::save(const std::string& to_file) const
{
    // PNG expects a ABGR format (RGBA in reverse)
    std::vector<uint32_t> converted(m_data, m_data + size());
    for (uint32_t& source_cx : converted) source_cx = pixel_abgr(pixel_rgba(source_cx));

    if (stbi_write_png(
        to_file.c_str(),
        m_width, m_height,
        channels,
        converted.data(),
        0))
        return true;
    else return false;
}

bool basic_renderer::save(uint32_t * to_buffer, const size_t size_in_bytes) const
{
    if (size_in_bytes != this->size_in_bytes() || !to_buffer) return false;

    // Bitmaps expects a ABGR format (RGBA in reverse)
    std::vector<uint32_t> converted(m_data, m_data + size());
    for (uint32_t& source_cx : converted) source_cx = pixel_abgr(pixel_rgba(source_cx));

    std::memcpy(to_buffer, converted.data(), size_in_bytes);

    return true;
}
