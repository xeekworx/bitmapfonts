// xwfont_generator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
    xeekworx::xwf_test("C:\\Windows\\Fonts\\arial.ttf", 18, 32, 2048, 0xFF00FF22, 0xFF333333);
    return 0;
}

