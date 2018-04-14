#include "renderer_nanovg.h"

// Using dav1d's glad extension library, with prefixes renamed so they do not 
// collide with glad definitions applications that use this bitmapfont library.
#include "xwfglad/xwfglad.h"

//#define NANOVG_GL3_IMPLEMENTATION // I expect the user of this library to do this
#include <nanovg.h>
#include <nanovg_gl.h>

using namespace xeekworx::bitmapfonts;

renderer_nanovg::renderer_nanovg(void * nvgcontext)
    : m_nvgcontext(nvgcontext)
{

}

renderer_nanovg::~renderer_nanovg()
{
    for (auto font : get_fonts()) {
        for (auto image : font.renderer_images)
            on_destroy_image(image);
    }
}

uintptr_t renderer_nanovg::on_create_image(const uint32_t * data, int32_t width, int32_t height, int32_t channels)
{
    int image = 0;
    if ((image = nvgCreateImageRGBA(
        reinterpret_cast<NVGcontext *>(m_nvgcontext),
        width, height, 0, (const unsigned char*)data
    )) == 0) {
        return reinterpret_cast<uintptr_t>(nullptr);
    }
    else return static_cast<uintptr_t>(image);
}

void renderer_nanovg::on_destroy_image(uintptr_t image)
{
    if (image) {
        nvgDeleteImage(
            reinterpret_cast<NVGcontext *>(m_nvgcontext),
            static_cast<int>(image)
        );
    }
}

void renderer_nanovg::on_draw_line(const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const color& clr) const
{
    if (m_nvgcontext) {
        NVGcontext * vg = reinterpret_cast<NVGcontext *>(m_nvgcontext);

        nvgBeginPath(vg);
        nvgMoveTo(vg, (float)x1, (float)y1);
        nvgLineTo(vg, (float)x2, (float)y2);
        nvgStrokeColor(vg, nvgRGBA(clr.r, clr.g, clr.b, clr.a));
        nvgStrokeWidth(vg, 1.f);
        nvgStroke(vg);
    }
}

void renderer_nanovg::on_draw_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const color& clr, const uint32_t thickness) const
{
    if (m_nvgcontext) {
        NVGcontext * vg = reinterpret_cast<NVGcontext *>(m_nvgcontext);

        nvgBeginPath(vg);
        nvgRect(vg, (float)x, (float)y, (float)w, (float)h);
        nvgStrokeWidth(vg, (float)thickness);
        nvgStrokeColor(vg, nvgRGBA(clr.r, clr.g, clr.b, clr.a));
        nvgStroke(vg);
    }
}

void renderer_nanovg::on_fill_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const color& clr) const
{
    if (m_nvgcontext) {
        NVGcontext * vg = reinterpret_cast<NVGcontext *>(m_nvgcontext);

        nvgBeginPath(vg);
        nvgRect(vg, (float)x, (float)y, (float)w, (float)h);
        nvgFillColor(vg, nvgRGBA(clr.r, clr.g, clr.b, clr.a));
        nvgFill(vg);
    }
}

void renderer_nanovg::on_draw_image(
    uintptr_t source_img,
    const int32_t source_x, const int32_t source_y,
    const int32_t source_w, const int32_t source_h,
    const int32_t x, const int32_t y,
    const int32_t width, const int32_t height,
    const color& modulation,
    const rotation direction
) const
{
    if (m_nvgcontext) {
        NVGcontext * vg = reinterpret_cast<NVGcontext *>(m_nvgcontext);

        float angle = 0.f;
        int destX = x, destY = y;
        switch (direction) {
        case rotation::right90degrees:
            destX += height;
            angle = (90.f * NVG_PI) / 180; // Radians
            break;
        case rotation::left90degrees:
            destY += width;
            angle = (-90.f * NVG_PI) / 180; // Radians
            break;
        }

        nvgSave(vg);
        nvgTranslate(vg, (float)destX, (float)destY);

        float ax = (float)width / (float)source_w;
        float ay = (float)height / (float)source_h;
        int original_w, original_h;
        nvgImageSize(vg, static_cast<int>(source_img), &original_w, &original_h);
        NVGpaint paint_img = nvgImagePattern(
            vg, (float)(-source_x * ax), (float)(-source_y * ay), (float)original_w*ax, (float)original_h*ay, 0.f, static_cast<int>(source_img), 1.f
        );

        nvgRotate(vg, angle);

        nvgBeginPath(vg);
        nvgRect(vg, 0.f, 0.f, (float)width, (float)height);
        paint_img.innerColor = nvgRGBA(
            modulation.r,
            modulation.g,
            modulation.b,
            modulation.a);
        nvgFillPaint(vg, paint_img);
        nvgFill(vg);

        nvgRestore(vg);
    }
}
