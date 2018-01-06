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
#include <memory>

#include "xwfont_api.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "image.h"
#include <pack.h>

namespace xeekworx {
    namespace bitmap_fonts {

        static constexpr size_t errorstr_length = 256;
        static char errorstr[errorstr_length] = {};
        static std::vector<image_ptr> images;

} }

XWFONTAPI const char * xeekworx::bitmap_fonts::get_error(void)
{
    return errorstr;
}

XWFONTAPI int xeekworx::bitmap_fonts::generate(const generate_config * config)
{
    int                     result = 0; // eventually the total number of page images generated
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

        // Reset page images vector:
        xeekworx::bitmap_fonts::images.clear();

        // Generate page images:
        for (size_t b = 0; b < bins.size(); ++b) {
            image_ptr page_image = std::make_unique<image>(bins[b].size.w, bins[b].size.h, config->background);

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

                page_image->draw_bitmap(
                    &slot->bitmap, 
                    rect.x + border_thickness + padding, 
                    rect.y + border_thickness + padding,
                    config->foreground);

                if (border_thickness) {
                    page_image->draw_rect(
                        rect.x + padding, rect.y + padding, 
                        rect.w - (padding * 2), rect.h - (padding * 2), 
                        border_thickness, config->border);
                }
            }

            std::stringstream filename_ss;
            filename_ss << "test." << std::setw(2) << std::setfill('0') << b << ".png";

            page_image->save(filename_ss.str());

            xeekworx::bitmap_fonts::images.push_back(page_image);
        }

        result = xeekworx::bitmap_fonts::images.size();
    }
    catch(std::string e) {
        strncpy(errorstr, e.c_str(), errorstr_length);
        result = 0;
    }

    // Clean-up:
    if(face) FT_Done_Face(face);
    if(library) FT_Done_FreeType(library);

    return result;
}
