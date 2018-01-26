![Xeekworx](http://xeekworx.com/images/github/xeekworx_logo.png) <br />
Bitmap Font API & Tools
===========

**This is an API and Tools to aid in use of custom Xeekworx Bitmap Fonts in games.**

You'll want to build freetype2 in the third-party directory before the xwfont_library & xwfont_generator projects can be built. I've created a Visual Studio 2017 configuration in the freetype projects folder.
A majority of the project is in C++, but exposed as exportable C functions for use in an upcoming C# GUI application.

WARNING: This project has just begun; it is not yet functional!

Here's an early sample of what's been accomplished so far:
![Early Sample](https://github.com/xeekworx/bitmapfonts/blob/master/images/early_sample.png)

Glyph bitmaps are rendered using freetype2 and copied into an image right side up or rotated 90 degrees so that they can be crammed efficiently. Currently only images files are saved, but eventually this glyph crammed image will be saved with another file (maybe json) with all of the glyph metrics.

Right now I'm working on the rendering for various backends (sdl, opengl, and nanovg).