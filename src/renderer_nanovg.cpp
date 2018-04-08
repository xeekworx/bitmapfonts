#include "renderer_nanovg.h"
#include <string>
#include <memory>
#include <SDL.h>
#include <glad\glad.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

using namespace xeekworx::bitmapfonts;

renderer_nanovg::renderer_nanovg()
    : m_nvgcontext(nullptr)
{

}

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

const xwf_font * renderer_nanovg::set_font(const xwf_font * font, font_style style)
{
    return renderer_base::set_font(font, style);
}

namespace xeekworx {
    namespace bitmapfonts {

        struct renderer_nanovg_testenv {
            SDL_Window * window = nullptr;
            SDL_GLContext glcontext = nullptr;
            NVGcontext * nvgcontext = nullptr;

            renderer_nanovg_testenv(int width, int height, bool hidden, color backgrd = color::black) {
                if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
                    throw std::string(SDL_GetError());

                // Configure OpenGL Attributes:
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
                SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
                SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

                if ((window = SDL_CreateWindow(
                    "xeekworx::bitmapfonts::test_environment",
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    width, height,
                    SDL_WINDOW_OPENGL | (hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN)
                )) == 0)
                    throw std::string(SDL_GetError());

                // Create OpenGL Context:
                if (NULL == (glcontext = SDL_GL_CreateContext(window))) {
                    throw std::string(SDL_GetError());
                }
                else SDL_GL_MakeCurrent(window, glcontext);

                // Initialize GLAD:
                if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
                    throw(std::string("Failed to initialize GLAD"));
                }

                // Create NanoVG Context:
                if (NULL == (nvgcontext = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_ANTIALIAS | NVG_DEBUG))) {
                    throw(std::string("Failed to create NVG Context"));
                }

                SDL_PumpEvents();

                glViewport(0, 0, width, height);
                glClearColor(
                    backgrd.r != 0 ? (float)backgrd.r / 255.f : 0.f,
                    backgrd.g != 0 ? (float)backgrd.g / 255.f : 0.f,
                    backgrd.b != 0 ? (float)backgrd.b / 255.f : 0.f,
                    backgrd.a != 0 ? (float)backgrd.a / 255.f : 0.f);
                glClear(GL_COLOR_BUFFER_BIT);
            }

            ~renderer_nanovg_testenv() {
                if (nvgcontext) nvgDeleteGL3(nvgcontext);
                if (glcontext) SDL_GL_DeleteContext(glcontext);
                if (window) SDL_DestroyWindow(window);
                SDL_Quit();
            }
        };

        static std::auto_ptr<renderer_nanovg_testenv> testenv; // auto_ptr so that if close_testenv isn't called eventually
    }
}

bool renderer_nanovg::setup_testenv(const int width, const int height, const bool hidden, char * error, const size_t error_max)
{
    bool result = true;

    try {
        testenv.reset(new renderer_nanovg_testenv(width, height, hidden, get_background()));
        m_nvgcontext = testenv->nvgcontext;
    }
    catch (const std::string& e) {
        if (error && error_max) strncpy(error, e.c_str(), error_max);
        result = false;
        close_testenv();
    }
    catch (...) {
        if (error && error_max) error[0] = 0;
        result = false;
        close_testenv();
    }

    return result;
}

bool renderer_nanovg::poll_testenv()
{
    SDL_Event e = {};
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            return false;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                return false;
            }
            break;
        }
    }

    if (testenv->window) SDL_GL_SwapWindow(testenv->window);

    color backcolor = get_background();
    glClearColor(
        backcolor.r != 0 ? (float)backcolor.r / 255.f : 0.f,
        backcolor.g != 0 ? (float)backcolor.g / 255.f : 0.f,
        backcolor.b != 0 ? (float)backcolor.b / 255.f : 0.f,
        backcolor.a != 0 ? (float)backcolor.a / 255.f : 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    return true;
}

void renderer_nanovg::close_testenv()
{
    if(is_testenv_setup()) testenv.reset();
}

bool renderer_nanovg::is_testenv_setup()
{
    return (testenv.get() ? true : false);
}

void renderer_nanovg::nanovg_begin()
{
    // Enable Blending:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Calculate Pixel Ratio for hi-dpi devices:
    int winWidth = 0, winHeight = 0, fbWidth = 0, fbHeight = 0;
    SDL_GetWindowSize(testenv->window, &winWidth, &winHeight);
    SDL_GL_GetDrawableSize(testenv->window, &fbWidth, &fbHeight);
    float pxRatio = (float)fbWidth / (float)winWidth;

    nvgBeginFrame(
        reinterpret_cast<NVGcontext *>(m_nvgcontext),
        winWidth,
        winHeight,
        pxRatio
    );
}

void renderer_nanovg::nanovg_end()
{
    nvgEndFrame(reinterpret_cast<NVGcontext *>(m_nvgcontext));
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
