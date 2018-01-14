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
#include "image.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <pack.h> // TODO: Switch to stb_rect_pack someday (maybe)
#include <utf8.h>

namespace xeekworx {
    namespace bitmap_fonts {

        static constexpr size_t errorstr_length = 256;
        static char errorstr[errorstr_length] = {};

        static uint32_t sample_background = image::transparent;

} }

XWFONTAPI const char * xeekworx::bitmap_fonts::get_error(void)
{
    return errorstr;
}

XWFONTAPI xeekworx::bitmap_fonts::xwf_font * xeekworx::bitmap_fonts::generate_font(const xwf_generation_config * config)
{
    xwf_font *              font = new xwf_font;
    const unsigned int      dpiX = 100, dpiY = 100;
    long                    size_px = config->font_size;
    FT_Error                error = 0;
    FT_Library              library = nullptr;
    FT_Face                 face = nullptr;
    FT_GlyphSlot            slot = nullptr;
    FT_Vector               pen = { 0, 0 };
    unsigned int            glyph_width = 0, glyph_height = 0;

    // Try-except is merely to facilitate a non-duplicated clean-up procedure
    try {
        // INITIALIZE FREETYPE & LOAD FONT FACE:
        if ((error = FT_Init_FreeType(&library))) throw std::string("Failed to initialize FreeType");
        if ((error = FT_New_Face(library, config->font_path, 0, &face))) throw std::string("Failed to create new Font Face from font path");
        if ((error = FT_Set_Char_Size(face, size_px * 64, 0, dpiX, dpiY))) throw std::string("Failed to set the Font size");

        slot = face->glyph;

        // DETERMINE GLYPH LOCATIONS IN IMAGE(S):
        // Using a third party api (rectpack2D) to determine glyph locations within
        // an image; this could also result in multiple images.
        // Note: this is just for locations and image dimensions, no actual image
        // generation happens here.

        // Derive from rectpack2D's structure and add some of our needed fields to
        // it (flipped will mean that the glyph should be rotated 90 degrees):
        struct rect_xywhf_glyph : rect_xywhf {
            unsigned long character;

            rect_xywhf_glyph() : rect_xywhf() {}
            rect_xywhf_glyph(unsigned long character, int x, int y, int width, int height) :
                rect_xywhf(x, y, width, height), character(character) {
                flipped = false;
            }
        };

        unsigned long rects_index = 0;
        std::vector<rect_xywhf_glyph> rects(config->end_char - config->begin_char);
        std::vector<rect_xywhf_glyph*> rect_ptrs(config->end_char - config->begin_char);
        std::vector<bin> bins; // Each bin is an image
        for (unsigned long n = config->begin_char; n < config->end_char; ++n)
        {
            if (!FT_Get_Char_Index(face, n)) continue; // No glyph image exists for this character

            error = FT_Load_Char(face, n, FT_LOAD_RENDER);
            if (error) continue;

            rects[rects_index] = rect_xywhf_glyph(n, 0, 0,
                slot->bitmap.width + (config->border_thickness * 2) + (config->padding * 2),
                slot->bitmap.rows + (config->border_thickness * 2) + (config->padding * 2));
            rect_ptrs[rects_index] = &rects[rects_index];
            ++rects_index;
        }
        // Resize the rect and rect pointer vectors to be just as large as they need to be
        rects.resize(rects_index);
        rect_ptrs.resize(rects_index);

        if (!pack((rect_xywhf*const*)rect_ptrs.data(), rects.size(), config->page_size, bins)) {
            throw std::string("Failed to pack glyphs");
        }

        // ALLOCATE THE NEW FONT:
        font->font_size = config->font_size;
        //font->line_height = face->size->metrics.height >> 6;
        font->line_height = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
        font->start_glyph_index = config->begin_char;
        font->glyph_indexes = new uint32_t[config->end_char - config->begin_char];
        font->num_glyph_indexes = config->end_char - config->begin_char;

        font->glyphs = new xwf_glyph[rects_index];
        font->num_glyphs = rects_index;

        font->images = new xwf_image[bins.size()];
        font->num_images = bins.size();
        for (uint32_t i = 0; i < font->num_images; ++i)
        {
            font->images[i].width = bins[i].size.w;
            font->images[i].height = bins[i].size.h;
            font->images[i].channels = image::channels;
            font->images[i].data = new uint32_t[font->images[i].width * font->images[i].height];
        }

        // GENERATE FONT (IMAGES & DATA):
        // This is where the actual rendering of the glyphs occurs.

        for (size_t b = 0; b < bins.size(); ++b)
        {
            // Create a temporary image object that has rendering capability:
            image_ptr page_image = std::make_shared<image>(bins[b].size.w, bins[b].size.h, config->background);

            // Iterate each glyph / rect pointer within the bin
            for (uint32_t i=0; i < bins[b].rects.size(); ++i)
            {
                const rect_xywhf_glyph& rect = *((rect_xywhf_glyph*)bins[b].rects[i]);

                error = FT_Load_Char(face, rect.character, FT_LOAD_RENDER);
                if (error) continue;

                if(rect.flipped)
                    page_image->draw_bitmap_rotated(
                        &slot->bitmap, 
                        rect.x + config->border_thickness + config->padding,
                        rect.y + config->border_thickness + config->padding,
                        config->foreground);
                else
                    page_image->draw_bitmap(
                        &slot->bitmap,
                        rect.x + config->border_thickness + config->padding,
                        rect.y + config->border_thickness + config->padding,
                        config->foreground);

                if (config->border_thickness) {
                    page_image->draw_rect(
                        rect.x + config->padding, rect.y + config->padding,
                        rect.w - (config->padding * 2), rect.h - (config->padding * 2),
                        config->border_thickness, config->border);
                }

                // Glyph data collection:
                font->glyph_indexes[rect.character - font->start_glyph_index] = i;
                font->glyphs[i].character = rect.character;
                font->glyphs[i].flipped = rect.flipped;
                font->glyphs[i].source_image = b;
                font->glyphs[i].source_x = rect.x + config->border_thickness + config->padding;
                font->glyphs[i].source_y = rect.y + config->border_thickness + config->padding;
                font->glyphs[i].source_w = !rect.flipped ? slot->bitmap.width : slot->bitmap.rows;
                font->glyphs[i].source_h = !rect.flipped ? slot->bitmap.rows : slot->bitmap.width;
                font->glyphs[i].advance_x = slot->advance.x >> 6;
                font->glyphs[i].advance_y = slot->advance.y >> 6;
                font->glyphs[i].bearing_left = slot->bitmap_left;
                font->glyphs[i].bearing_top = slot->bitmap_top;
            }

            std::stringstream filename;
            filename << "test." << std::setw(2) << std::setfill('0') << b << ".png";
            page_image->save(filename.str());

            page_image->save(font->images[b].data, font->images[b].width * font->images[b].channels * font->images[b].height);
        }
    }
    catch(std::string e) {
        strncpy(errorstr, e.c_str(), errorstr_length);
        xeekworx::bitmap_fonts::delete_font(font);
        font = nullptr;
    }

    // Clean-up:
    if(face) FT_Done_Face(face);
    if(library) FT_Done_FreeType(library);

    return font;
}

XWFONTAPI int xeekworx::bitmap_fonts::delete_font(xwf_font * font)
{
    if (font) {

        if (font->glyph_indexes) {
            delete[] font->glyph_indexes;
            font->glyph_indexes = nullptr;
        }

        if (font->glyphs) {
            delete[] font->glyphs;
            font->glyphs = nullptr;
        }
        
        if (font->images) {
            for (uint32_t i = 0; i < font->num_images; ++i) {
                if (!font->images[i].data) continue;
                delete[] font->images[i].data;
                font->images[i].data = nullptr;
            }

            delete[] font->images;
            font->images = nullptr;
        }

        delete font;
        return 0;
    }
    else {
        return -1;
    }
}



XWFONTAPI void xeekworx::bitmap_fonts::set_sample_background(uint32_t background)
{
    sample_background = background;
}

namespace xeekworx { namespace bitmap_fonts {
        static int render_sample_internal(
            const xwf_font * font,
            const uint32_t * text, const int length,
            int& width, int& height,
            bool measure_only = false)
        {
            // Sanity checks...
            if (!font || !font->glyphs || !font->images) return -1;
            if (length <= 0) return -1;

            // Convert the font's images / pages into image objects so they're easier
            // to work with:
            std::vector<image> images; // No images will be here with measure_only
            if (!measure_only) {
                for (unsigned i = 0; i < font->num_images; ++i) {
                    images.push_back(image(font->images[i].data, font->images[i].width, font->images[i].height, true));
                }
            }

            // Create sample image, only if not just measuring:
            std::unique_ptr<image> sample_image;
            if (!measure_only) sample_image = std::make_unique<image>(width, height, sample_background);

            // RENDERING & MEASUREMENTS:
            // To get the actual width of rendered text we're going to have to simulate
            // drawing the text so that the advance width and bearing are calculated.
            //
            // TODO: If I assume text is in UTF8 I should be converting that to 32bit 
            //       codepoints or really this function should take UTF-32 and other 
            //       functions doing some conversion.
            int32_t measure_width = 0, measure_height = 0;
            for (int32_t i = 0, x = 0, y = font->font_size, farthestX = 0, farthestY = 0; i < length; ++i) {
                // Handle new lines first:
                if (text[i] == (uint32_t) '\n') {
                    x = 0;
                    y += font->line_height;
                    continue;
                }

                if (text[i] < font->start_glyph_index ||
                    text[i] >= font->start_glyph_index + font->num_glyph_indexes)
                    continue; // TODO: Draw a special flyph for non-existent glyphs

                uint32_t glyph_index = font->glyph_indexes[text[i] + font->start_glyph_index];
                if (glyph_index >= font->num_glyphs)
                    continue; // TODO: Warn about something wrong with the font

                const xwf_glyph& glyph = font->glyphs[font->glyph_indexes[text[i] + font->start_glyph_index]];

                // Text Measurement farthest extents:
                if (measure_only) {
                    farthestX = (x + glyph.bearing_left) + (!glyph.flipped ? glyph.source_w : glyph.source_h);
                    farthestY = (y - glyph.bearing_top) + (!glyph.flipped ? glyph.source_h : glyph.source_w);
                    if (farthestX > measure_width) measure_width = farthestX;
                    if (farthestY > measure_height) measure_height = farthestY;
                }

                // Actual rendering here, if not just measuring:
                if (!measure_only) {
                    if (glyph.flipped) {
                        sample_image->draw_bitmap_rotated
                        (
                            images[glyph.source_image],
                            glyph.source_x,
                            glyph.source_y,
                            glyph.source_w,
                            glyph.source_h,
                            x + glyph.bearing_left,
                            y - glyph.bearing_top
                        );
                    }
                    else
                        sample_image->draw_bitmap
                        (
                            images[glyph.source_image],
                            glyph.source_x,
                            glyph.source_y,
                            glyph.source_w,
                            glyph.source_h,
                            x + glyph.bearing_left,
                            y - glyph.bearing_top
                        );
                }

                // Move the pen:
                x += glyph.advance_x;
            }

            if (!measure_only) sample_image->save("sample.png");
            else {
                width = measure_width;
                height = measure_height;
            }
            return 0;
        }
    }
}

XWFONTAPI int xeekworx::bitmap_fonts::render_sample_utf8(
    const xwf_font * font,
    const char * text, int length,
    int width, int height)
{
    length = length < 0 ? std::strlen(text) : length;
    std::string in_text(text, text + length);

    // Convert it to utf-32
    std::string::iterator end_it = utf8::find_invalid(in_text.begin(), in_text.end());
    std::vector<uint32_t> out_utf32;
    utf8::utf8to32(in_text.begin(), end_it, std::back_inserter(out_utf32));
    
    return render_sample_internal(font, out_utf32.data(), out_utf32.size(), width, height, false);
}

XWFONTAPI int xeekworx::bitmap_fonts::render_sample_utf16(
    const xwf_font * font,
    const wchar_t * text, int length,
    int width, int height)
{
    length = length < 0 ? std::wcslen(text) : length;
    std::wstring in_text(text, text + length);

    // Convert it to utf-8
    std::string out_utf8;
    utf8::utf16to8(in_text.begin(), in_text.end(), std::back_inserter(out_utf8));

    // Convert it to utf-32
    std::string::iterator end_it = utf8::find_invalid(out_utf8.begin(), out_utf8.end());
    std::vector<uint32_t> out_utf32;
    utf8::utf8to32(out_utf8.begin(), end_it, std::back_inserter(out_utf32));

    return render_sample_internal(font, out_utf32.data(), out_utf32.size(), width, height, false);
}

XWFONTAPI int xeekworx::bitmap_fonts::measure_sample_utf8(
    const xwf_font * font,
    const char * text, int length,
    int * out_width, int * out_height)
{
    length = length < 0 ? std::strlen(text) : length;
    std::string in_text(text, text + length);

    // Convert it to utf-32
    std::string::iterator end_it = utf8::find_invalid(in_text.begin(), in_text.end());
    std::vector<uint32_t> out_utf32;
    utf8::utf8to32(in_text.begin(), end_it, std::back_inserter(out_utf32));

    int tmp_w = 0, tmp_h = 0;
    int result = render_sample_internal(font, out_utf32.data(), out_utf32.size(), tmp_w, tmp_h, true);

    if (out_width) *out_width = tmp_w;
    if (out_height) *out_height = tmp_h;

    return result;
}

XWFONTAPI int xeekworx::bitmap_fonts::measure_sample_utf16(
    const xwf_font * font,
    const wchar_t * text, int length,
    int * out_width, int * out_height)
{
    length = length < 0 ? std::wcslen(text) : length;
    std::wstring in_text(text, text + length);

    // Convert it to utf-8
    std::string out_utf8;
    utf8::utf16to8(in_text.begin(), in_text.end(), std::back_inserter(out_utf8));

    // Convert it to utf-32
    std::string::iterator end_it = utf8::find_invalid(out_utf8.begin(), out_utf8.end());
    std::vector<uint32_t> out_utf32;
    utf8::utf8to32(out_utf8.begin(), end_it, std::back_inserter(out_utf32));

    int tmp_w = 0, tmp_h = 0;
    int result = render_sample_internal(font, out_utf32.data(), out_utf32.size(), tmp_w, tmp_h, true);

    if (out_width) *out_width = tmp_w;
    if (out_height) *out_height = tmp_h;

    return result;
}