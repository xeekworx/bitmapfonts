// xwfont_generator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
    xeekworx::bitmap_fonts::xwf_generation_config config;
    config.font_path = "C:\\Windows\\Fonts\\arial.ttf";
    config.font_size = 32;
    config.begin_char = 0;
    config.end_char = 255;
    config.foreground = 0xFF0000FF;
    config.background = 0xFFFFFF00;
    config.border = 0x00FF00FF;
    //config.border_thickness = 1;
    //config.padding = 3;
    //config.page_size = 1024;

    xeekworx::bitmap_fonts::xwf_font * font = xeekworx::bitmap_fonts::generate_font(&config);
    if (!font) return -1;
    else {
        xeekworx::bitmap_fonts::set_sample_background(0xFFFFFF00);

        std::wstring sample_text = L"MWll!space space_./?;\1234567890";
        int sample_w = 800, sample_h = 100;
        xeekworx::bitmap_fonts::measure_sample_utf16(font, sample_text.c_str(), -1, &sample_w, &sample_h);
        xeekworx::bitmap_fonts::render_sample_utf16(font, sample_text.c_str(), -1, sample_w, sample_h);

        xeekworx::bitmap_fonts::delete_font(font);
    }

    return 0;
}

