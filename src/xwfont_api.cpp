// xwfont_api.cpp : Defines the exported functions for the DLL application.
//

#include <SDKDDKVer.h>      // Including SDKDDKVer.h defines the highest available Windows platform.
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include "xwfont_api.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT // Be sure to remove _CRT_SECURE_NO_WARNINGS from the project's properties when nothings merges fixes into stb
#include <stb_image.h>
#include <stb_image_write.h>

#include <pack.h>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace xeekworx {
    namespace bitmap_fonts {

        static constexpr size_t errorstr_length = 256;
        static char errorstr[errorstr_length] = {};

        inline uint32_t rgba_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
            return ((r)+(g << 8) + (b << 16) + (a << 24));
        }

        inline uint32_t bgra_to_html(uint32_t color) {
            return ((((color >> 16) & 0xff)) + (((color >> 8) & 0xff) << 8) + (((color) & 0xff) << 16) + (((color >> 24) & 0xff) << 24));
        }

        // Converts from an html ordered color to what xwf uses internally (BGRA)
        inline uint32_t html_color(uint32_t webhex) {
            return (((webhex >> 16) & 0xff)) + (((webhex >> 8) & 0xff) << 8) + (((webhex) & 0xff) << 16) + (((webhex >> 24) & 0xff) << 24);
        }

        inline uint32_t html_color_ex(uint32_t webhex, uint8_t alpha) {
            return (((webhex >> 16) & 0xff)) + (((webhex >> 8) & 0xff) << 8) + (((webhex) & 0xff) << 16) + (alpha << 24);
        }

        static uint32_t blend_colors(uint32_t foreground, uint32_t background);
        static void draw_hline(const int32_t x1, const int32_t x2, const int32_t y, uint32_t html_color, image& destination_img);
        static void draw_vline(const int32_t x, const int32_t y1, const int32_t y2, uint32_t html_color, image& destination_img);
        static void draw_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, uint32_t html_color, const uint32_t thickness, image& destination_img);
        static void draw_bitmap(const FT_Bitmap* ftbitmap, const int32_t x, const int32_t y, uint32_t foreground, image& destination_img);
} }

XWFONTAPI const char * xeekworx::bitmap_fonts::get_error(void)
{
    return errorstr;
}

XWFONTAPI int xeekworx::bitmap_fonts::generate(const generate_config * config)
{
    int                     result = 0;
    unsigned int            dpiX = 100, dpiY = 100;
    long                    size_px = config->font_size;
    FT_Error                error = 0;
    FT_Library              library = nullptr;
    FT_Face                 face = nullptr;
    FT_GlyphSlot            slot = nullptr;
    FT_Vector               pen = { 0, 0 };
    unsigned int            glyph_width = 0, glyph_height = 0;
    unsigned int            image_w = config->page_size, image_h = config->page_size;
    FT_Int                  border_thickness = config->border_thickness;
    unsigned int            padding = config->padding;

    // Try-except is merely to facilitate a non-duplicated clean-up procedure
    try {
        if ((error = FT_Init_FreeType(&library))) throw std::string("Failed to initialize FreeType");
        if ((error = FT_New_Face(library, config->font_path, 0, &face))) throw std::string("Failed to create new Font Face from font path");
        if ((error = FT_Set_Char_Size(face, size_px * 64, 0, dpiX, dpiY))) throw std::string("Failed to set the Font size");

        slot = face->glyph;

        struct rect_xywhf_glyph : rect_xywhf {
            unsigned long character;

            rect_xywhf_glyph() : rect_xywhf() {}
            rect_xywhf_glyph(unsigned long character, int x, int y, int width, int height) :
                rect_xywhf(x, y, width, height), character(character) {
                flipped = false;
            }
        };

        std::vector<rect_xywhf_glyph> rects(config->end_char - config->begin_char);
        std::vector<rect_xywhf_glyph*> ptr_rects(config->end_char - config->begin_char);
        std::vector<bin> bins;
        for (unsigned long n = config->begin_char, r = 0; n < config->end_char; ++n, ++r) {
            error = FT_Load_Char(face, n, FT_LOAD_RENDER);
            if (error) continue;

            rects[r] = rect_xywhf_glyph(n, 0, 0, 
                slot->bitmap.width + (border_thickness * 2) + (padding * 2), 
                slot->bitmap.rows + (border_thickness * 2) + (padding * 2));
            ptr_rects[r] = &rects[r];
        }
        if (!pack((rect_xywhf*const*)ptr_rects.data(), rects.size(), image_w, bins)) {
            throw std::string("Failed to pack glyphs");
        }

        for (size_t b = 0; b < bins.size(); ++b) {
            image page_image(bins[b].size.w, bins[b].size.h, html_color(config->background));

            for (const rect_xywhf* prect : bins[b].rects)
            {
                const rect_xywhf_glyph& rect = *((rect_xywhf_glyph*)prect);

                if (rect.flipped) {
                    FT_Matrix matrix;
                    double angle = (90.0 / 360) * 3.14159 * 2;
                    matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
                    matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
                    matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
                    matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
                    FT_Set_Transform(face, &matrix, &pen);
                }
                else FT_Set_Transform(face, NULL, &pen);

                error = FT_Load_Char(face, rect.character, FT_LOAD_RENDER);
                if (error) continue;

                draw_bitmap(
                    &slot->bitmap, 
                    rect.x + border_thickness + padding, 
                    rect.y + border_thickness + padding,
                    config->foreground,
                    page_image);

                if (border_thickness) {
                    draw_rect(rect.x + padding, rect.y + padding, rect.w - (padding * 2), rect.h - (padding * 2), config->border, border_thickness, page_image);
                }
            }

            std::stringstream filename_ss;
            filename_ss << "test." << std::setw(2) << std::setfill('0') << b << ".png";

            stbi_write_png(filename_ss.str().c_str(), page_image.w, page_image.h, page_image.channels, page_image.pixels, 0);
        }
    }
    catch(std::string e) {
        strncpy(errorstr, e.c_str(), errorstr_length);
        result = 1;
    }

    // Clean-up:
    if(face) FT_Done_Face(face);
    if(library) FT_Done_FreeType(library);

    return result;
}

// Takes html colors and spits out BGRA
static uint32_t xeekworx::bitmap_fonts::blend_colors(uint32_t foreground, uint32_t background)
{
    unsigned char dest_blue = (background) & 0xff;
    unsigned char dest_green = (background >> 8) & 0xff;
    unsigned char dest_red = (background >> 16) & 0xff;
    unsigned char dest_alpha = (background >> 24) & 0xff;

    if (dest_alpha == 0) return html_color(foreground);

    unsigned char source_blue = (foreground) & 0xff;
    unsigned char source_green = (foreground >> 8) & 0xff;
    unsigned char source_red = (foreground >> 16) & 0xff;
    unsigned char source_alpha = (foreground >> 24) & 0xff;

    uint8_t final_alpha = source_alpha + dest_alpha * (255 - source_alpha) / 255;
    unsigned char final_red = (source_red * source_alpha + dest_red * dest_alpha * (255 - source_alpha) / 255) / final_alpha;
    unsigned char final_green = (source_green * source_alpha + dest_green * dest_alpha * (255 - source_alpha) / 255) / final_alpha;
    unsigned char final_blue = (source_blue * source_alpha + dest_blue * dest_alpha * (255 - source_alpha) / 255) / final_alpha;

    return rgba_color(final_red, final_green, final_blue, final_alpha);
}

static void xeekworx::bitmap_fonts::draw_hline(const int32_t x1, const int32_t x2, const int32_t y, uint32_t html_color, image& destination_img)
{
    if (y >= 0 && y < destination_img.h)
        for (int32_t x = x1; x < x2; ++x)
            if (x >= 0 && x < destination_img.w)
                destination_img.to_point(x, y, blend_colors(html_color, destination_img.from_point(x, y)));
}

static void xeekworx::bitmap_fonts::draw_vline(const int32_t x, const int32_t y1, const int32_t y2, uint32_t html_color, image& destination_img)
{
    if (x >= 0 && x < destination_img.w)
        for (int32_t y = y1; y < y2; ++y)
            if (y>= 0 && y < destination_img.h)
                destination_img.to_point(x, y, blend_colors(html_color, destination_img.from_point(x, y)));
}

static void xeekworx::bitmap_fonts::draw_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, uint32_t html_color, const uint32_t thickness, image& destination_img)
{
    // left
    for (uint32_t t = 0; t < thickness; ++t)
        draw_vline(x + t, y, y + h, html_color, destination_img);

    // top
    for (uint32_t t = 0; t < thickness; ++t)
        draw_hline(x+thickness, x + w, y+t, html_color, destination_img);

    // right
    for (uint32_t t = 0; t < thickness; ++t)
        draw_vline((x + w - 1) - t, y + thickness, y + h, html_color, destination_img);

    // bottom
    for (uint32_t t = 0; t < thickness; ++t)
        draw_hline(x+thickness, x + w - thickness, (y + h - 1) - t, html_color, destination_img);
}

static void xeekworx::bitmap_fonts::draw_bitmap(const FT_Bitmap* ftbitmap, const int32_t x, const int32_t y, uint32_t foreground, image& destination_img)
{
    int32_t  i, j, p, q;
    int32_t  x_max = x + ftbitmap->width;
    int32_t  y_max = y + ftbitmap->rows;

    for (i = x, p = 0; i < x_max; i++, p++)
    {
        for (j = y, q = 0; j < y_max; j++, q++)
        {
            if (i < 0 || j < 0 || i >= destination_img.w || j >= destination_img.h) continue;

            destination_img.to_point(i, j, 
                blend_colors(
                    html_color_ex(foreground, ftbitmap->buffer[q * ftbitmap->width + p]), // Target Color
                    bgra_to_html(destination_img.from_point(i, j)) // Background Color
                )
            );
        }
    }
}