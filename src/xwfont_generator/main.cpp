// xwfont_generator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
    xeekworx::xwf_test("C:\\Windows\\Fonts\\arial.ttf", 200, 0, 256, 0xFF00FF22, 0xFFFFFFFF);
    return 0;
}

