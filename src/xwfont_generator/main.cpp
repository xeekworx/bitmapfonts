// xwfont_generator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
    xeekworx::bitmap_fonts::generate_config config;
    config.font_path = "C:\\Windows\\Fonts\\ariali.ttf";
    config.font_size = 32;
    config.begin_char = 0;
    config.end_char = 1024;
    config.foreground = 0xFF0000FF;
    config.background = 0x0000FFFF;
    config.border = 0x00FF00FF;
    //config.border_thickness = 1;
    //config.padding = 3;
    //config.page_size = 1024;

    return xeekworx::bitmap_fonts::generate(&config);
}

