// xwfont_api.cpp : Defines the exported functions for the DLL application.
//

#include <SDKDDKVer.h>      // Including SDKDDKVer.h defines the highest available Windows platform.
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>

#include "xwfont_api.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT // Be sure to remove _CRT_SECURE_NO_WARNINGS from the project's properties when nothings merges fixes into stb
#include <stb_image.h>
#include <stb_image_write.h>

#include <pack.h>

#include <ft2build.h>
#include FT_FREETYPE_H

static constexpr size_t xwf_errorstr_length = 256;
static char xwf_errorstr[xwf_errorstr_length] = {};

static void draw_rect(const FT_Int x, const FT_Int y, const FT_Int width, const FT_Int height, uint32_t html_color, const FT_Int thickness, std::vector<uint32_t>& image, const FT_Int image_width, const FT_Int image_height);
static void draw_bitmap(const FT_Bitmap* bitmap, const FT_Int x, const FT_Int y, std::vector<uint32_t>& image, const FT_Int image_width, const FT_Int image_height, uint32_t foreground);

XWFONTAPI const char * xeekworx::xwf_geterror(void)
{
    return xwf_errorstr;
}

XWFONTAPI int xeekworx::xwf_test(const char * font_path, long font_size, unsigned long start_char, unsigned long end_char, uint32_t foreground, uint32_t background)
{
    int                     result = 0;
    unsigned int            dpiX = 100, dpiY = 100;
    long                    size_px = font_size;
    FT_Error                error = 0;
    FT_Library              library = nullptr;
    FT_Face                 face = nullptr;
    FT_GlyphSlot            slot = nullptr;
    FT_Vector               pen = { 0, 0 };
    unsigned int            glyph_width = 0, glyph_height = 0;
    unsigned int            image_w = 1024, image_h = 1024;
    std::vector<uint32_t>   image(image_w * image_h, html_color(background));
    FT_Int                  border = 0;

    // Try-except is merely to facilitate a non-duplicated clean-up procedure
    try {
        if ((error = FT_Init_FreeType(&library))) throw std::string("Failed to initialize FreeType");
        if ((error = FT_New_Face(library, font_path, 0, &face))) throw std::string("Failed to create new Font Face from font path");
        if ((error = FT_Set_Char_Size(face, size_px * 64, 0, dpiX, dpiY))) throw std::string("Failed to set the Font size");

        slot = face->glyph;

        struct rect_xywhf_glyph : rect_xywhf {
            unsigned long character;

            rect_xywhf_glyph() : rect_xywhf() {}
            rect_xywhf_glyph(unsigned long character, int x, int y, int width, int height) : 
                rect_xywhf(x, y, width, height), character(character) { }
        };

        std::vector<rect_xywhf_glyph> rects(end_char - start_char);
        std::vector<rect_xywhf_glyph*> ptr_rects(end_char - start_char);
        std::vector<bin> bins;
        for (unsigned long n = start_char, r = 0; n < end_char; ++n, ++r) {
            error = FT_Load_Char(face, n, FT_LOAD_RENDER);
            if (error) continue;

            rects[r] = rect_xywhf_glyph(n, 0, 0, slot->bitmap.width + (border * 2), slot->bitmap.rows + (border * 2));
            ptr_rects[r] = &rects[r];
        }
        if (!pack((rect_xywhf*const*)ptr_rects.data(), rects.size(), image_w, bins)) {
            throw std::string("Failed to pack glyphs");
        }
        else {
            image_w = bins[0].size.w;
            image_h = bins[0].size.h;
        }

        for (const rect_xywhf_glyph& rect : rects)
        {
            glyph_width = slot->bitmap.width;
            glyph_height = slot->bitmap.rows;

            pen.x = 0;
            pen.y = 0;
            //FT_Set_Transform(face, NULL, &pen);

            if (rect.flipped) {
                FT_Matrix matrix;
                double angle = (90.0 / 360) * 3.14159 * 2;
                matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
                matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
                matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
                matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
                FT_Set_Transform(face, &matrix, NULL);
            }
            else {
                FT_Set_Transform(face, NULL, &pen);
            }

            error = FT_Load_Char(face, rect.character, FT_LOAD_RENDER);
            if (error) continue;

            draw_bitmap(&slot->bitmap, rect.x + border, rect.y + border, image, (FT_Int)image_w, (FT_Int)image_h, foreground);
            if (border) {
                draw_rect(rect.x, rect.y, rect.w, rect.h, 0xac000000, border, image, image_w, image_h);
            }
        }

        //FT_Int x = 0, y = 0;
        //for (unsigned long n = start_char; n < end_char; ++n) {
        //    FT_Set_Transform(face, NULL, &pen);

        //    error = FT_Load_Char(face, n, FT_LOAD_RENDER);
        //    if (error) continue;

        //    glyph_width = slot->bitmap.width;
        //    glyph_height = slot->bitmap.rows;

        //    if (x + 60 + (border * 2) /* frame */ >= image_w) { x = 0; y += 60 + (border * 2) /* frame */; }

        //    draw_bitmap(&slot->bitmap, x + border, y + border, image, (FT_Int) image_w, (FT_Int) image_h, foreground);
        //    draw_rect(x, y, glyph_width + border * 2, glyph_height + border * 2, 0xac000000, border, image, image_w, image_h);

        //    x += 60 + 2 /* frame */;
        //}

        stbi_write_png("test.png", image_w, image_h, 4, image.data(), 0);
    }
    catch(std::string e) {
        strncpy(xwf_errorstr, e.c_str(), xwf_errorstr_length);
        result = 1;
    }

    // Clean-up:
    if(face) FT_Done_Face(face);
    if(library) FT_Done_FreeType(library);

    return result;
}

// Takes html colors and spits out BGRA
static uint32_t blend_colors(uint32_t foreground, uint32_t background)
{
    unsigned char dest_blue = (background) & 0xff;
    unsigned char dest_green = (background >> 8) & 0xff;
    unsigned char dest_red = (background >> 16) & 0xff;
    unsigned char dest_alpha = (background >> 24) & 0xff;

    if (dest_alpha == 0) return xeekworx::html_color(foreground);

    unsigned char source_blue = (foreground) & 0xff;
    unsigned char source_green = (foreground >> 8) & 0xff;
    unsigned char source_red = (foreground >> 16) & 0xff;
    unsigned char source_alpha = (foreground >> 24) & 0xff;

    uint8_t final_alpha = source_alpha + dest_alpha * (255 - source_alpha) / 255;
    unsigned char final_red = (source_red * source_alpha + dest_red * dest_alpha * (255 - source_alpha) / 255) / final_alpha;
    unsigned char final_green = (source_green * source_alpha + dest_green * dest_alpha * (255 - source_alpha) / 255) / final_alpha;
    unsigned char final_blue = (source_blue * source_alpha + dest_blue * dest_alpha * (255 - source_alpha) / 255) / final_alpha;

    return xeekworx::rgba_color(final_red, final_green, final_blue, final_alpha);
}

static void draw_hline(const FT_Int x1, const FT_Int x2, const FT_Int y, uint32_t html_color, std::vector<uint32_t>& image, const FT_Int image_width, const FT_Int image_height)
{
    if (y >= 0 && y < image_height)
        for (FT_Int x = x1; x < x2; ++x)
            if (x >= 0 && x < image_width)
                image[x + image_width * y] = blend_colors(html_color, image[x + image_width * y]);
}

static void draw_vline(const FT_Int x, const FT_Int y1, const FT_Int y2, uint32_t html_color, std::vector<uint32_t>& image, const FT_Int image_width, const FT_Int image_height)
{
    if (x >= 0 && x < image_width)
        for (FT_Int y = y1; y < y2; ++y)
            if (y>= 0 && y < image_height)
                image[x + image_width * y] = blend_colors(html_color, image[x + image_width * y]);
}

static void draw_rect(const FT_Int x, const FT_Int y, const FT_Int width, const FT_Int height, uint32_t html_color, const FT_Int thickness, std::vector<uint32_t>& image, const FT_Int image_width, const FT_Int image_height)
{
    // left
    for (FT_Int t = 0; t < thickness; ++t)
        draw_vline(x + t, y, y + height, html_color, image, image_width, image_height);

    // top
    for (FT_Int t = 0; t < thickness; ++t)
        draw_hline(x+thickness, x + width, y+t, html_color, image, image_width, image_height);

    // right
    for (FT_Int t = 0; t < thickness; ++t)
        draw_vline((x + width - 1) - t, y + thickness, y + height, html_color, image, image_width, image_height);

    // bottom
    for (FT_Int t = 0; t < thickness; ++t)
        draw_hline(x+thickness, x + width - thickness, (y + height - 1) - t, html_color, image, image_width, image_height);
}

static void draw_bitmap(const FT_Bitmap* bitmap, const FT_Int x, const FT_Int y, std::vector<uint32_t>& image, const FT_Int image_width, const FT_Int image_height, uint32_t foreground)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;

    for (i = x, p = 0; i < x_max; i++, p++)
    {
        for (j = y, q = 0; j < y_max; j++, q++)
        {
            if (i < 0 || j < 0 || i >= image_width || j >= image_height) continue;

            uint32_t background = xeekworx::bgra_to_html(image[i + image_width * j]);
            uint32_t target_color = xeekworx::html_color_ex(foreground, bitmap->buffer[q * bitmap->width + p]);
            image[i + image_width * j] = blend_colors(target_color, background);
        }
    }
}