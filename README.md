![Xeekworx](http://xeekworx.com/images/github/xeekworx_logo.png) <br />
Bitmap Font API & Tools
===========

**This is an API and Tools to aid in use of custom Xeekworx Bitmap Fonts in games.**

You'll want to build freetype2 in the third-party directory before the xwfont projects can be built. I've created a Visual Studio 2017 configuration in the freetype projects folder.
A majority of the projects are in C++, but exposed as exportable C functions for use in an upcoming C# GUI application (XWFONT_API).

WARNING: This project has just begun so it may not be completely functional! The font generator, NanoVG renderer, and SDL2 renderer are functional. Currently fonts have to be generated before rendering because saving the font to a file is not yet supported.

## What's done so far?
Here's an early sample of what's been accomplished so far:
![Early Sample](https://github.com/xeekworx/bitmapfonts/blob/master/images/early_sample.png)

The font generator is nearly complete, but functional if generated and used in memory. At some point the xwf_font_generator project will turn into a command-line tool used to generate font files that can be loaded for the renderer. This is not yet implemented. Also, the font generator is implemented in mostly C so that it can be easily imported in C# for future tools.

Glyph bitmaps are rendered using freetype2 and copied into an image right side up or rotated 90 degrees so that they can be crammed efficiently. Currently only images files are saved, but eventually this glyph crammed image will be saved with another file (maybe json) with all of the glyph metrics.

Right now I'm working on the rendering for various rendering backends (sdl2, opengl, and nanovg). The SDL2 and NanoVG renderers are complete and comes with utility functions to start a test environment.

## Using the Font Generator & Renderer:
You'll see a similar example in the xwf_demo project in this repository.

```cpp
#include <string>
#define XWFONT_RENDERER_SDL // Choose your font renderer
#include <xwfont.h>

int main()
{
    // Configure the bitmapfont we want to generate:
    xeekworx::bitmapfonts::xwf_generation_config config;
    config.font_path = "C:\\Windows\\Fonts\\tahoma.ttf";
    config.font_size = 10;
    config.begin_char = 0;
    config.end_char = 1024;
    config.foreground = 0xFFFFFFFF;
    config.background = 0x00000000;
    config.border = 0x00FF20FF;
    config.border_thickness = 0;
    config.padding = 3;
    config.page_size = 1024; // Width, Height of the bitmaps
    int sample_w = 1024, sample_h = 900; // Test environment dimentions
 
    // Generate the font:
    xeekworx::bitmapfonts::xwf_font * font = xeekworx::bitmapfonts::generate_font(&config);
	if (!font) return -1; // Failed
	
    // Create the font renderer, using SDL2 as chosen by the #define above:
    xeekworx::bitmapfonts::renderer font_renderer;
    // Startup the test environment (creates a window, etc.)
    font_renderer.setup_testenv(sample_w, sample_h, false, NULL, 0);
    
    std::string sample_text = "Hello World!";	// Text to render (can also be wstring)
    font_renderer.set_font(font);		// Set the rendering font
    font_renderer.set_foreground(0xFF0000FF);	// Foreground color (color modulation used)
        
    // Using poll_testenv like this keeps the test window open until it's
    // closed or ESC is pressed.
    while (font_renderer.poll_testenv()) {
        font_renderer.draw(
	        sample_text.c_str(), 
	        -1,	// Text length (-1 to detect)
	        150,	// Draw at X coordinate
	        0,	// Draw at Y coordinate
	        300,	// Width of textbox if in textbox mode
	        500,	// Height of textbox if in textbox mode
	        25	// Padding (causes a margin around the text)
        );
    }
 
    // Clean-up:
    xeekworx::bitmapfonts::delete_font(font);

    return 0;
}

```
