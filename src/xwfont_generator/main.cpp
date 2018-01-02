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
    config.foreground = 0xFF00FF22;
    config.background = 0xFF333333;
    //config.border = 0xFF000000;
    //config.border_thickness = 1;
    //config.padding = 3;
    //config.page_size = 1024;

    return xeekworx::bitmap_fonts::generate(&config);
}

