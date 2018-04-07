#include "renderer_sdl.h"
#include "image.h"
#include <string>
#include <memory>
#include <SDL.h>

using namespace xeekworx::bitmapfonts;

renderer_sdl::renderer_sdl()
    : m_sdl_renderer(nullptr)
{

}

renderer_sdl::renderer_sdl(void * sdl_renderer)
    : m_sdl_renderer(sdl_renderer)
{

}

renderer_sdl::~renderer_sdl()
{
    for (auto font : get_fonts()) {
        for (auto image : font.renderer_images)
            on_destroy_image(image);
    }
}

const xwf_font * renderer_sdl::set_font(const xwf_font * font, font_style style)
{
    return renderer_base::set_font(font, style);
}

namespace xeekworx {
    namespace bitmapfonts {

        struct renderer_sdl_testenv {
            SDL_Window * window = nullptr;
            SDL_Renderer * renderer = nullptr;

            renderer_sdl_testenv(int width, int height, bool hidden) {
                if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
                    throw std::string(SDL_GetError());

                if ((window = SDL_CreateWindow(
                    "xeekworx::bitmapfonts::test_environment",
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    width, height,
                    hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN
                )) == 0)
                    throw std::string(SDL_GetError());

                if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == 0)
                    throw std::string(SDL_GetError());

                SDL_PumpEvents();
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderClear(renderer);
            }

            ~renderer_sdl_testenv() {
                if (renderer) SDL_DestroyRenderer(renderer);
                if (window) SDL_DestroyWindow(window);
                SDL_Quit();
            }
        };

        static std::auto_ptr<renderer_sdl_testenv> testenv; // auto_ptr so that if close_testenv isn't called eventually
    }
}

bool renderer_sdl::setup_testenv(const int width, const int height, const bool hidden, char * error, const size_t error_max)
{
    bool result = true;

    try {
        testenv.reset(new renderer_sdl_testenv(width, height, hidden));
        m_sdl_renderer = testenv->renderer;
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

bool renderer_sdl::poll_testenv()
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

    SDL_RenderPresent(reinterpret_cast<SDL_Renderer *>(m_sdl_renderer));

    return true;
}

void renderer_sdl::close_testenv()
{
    if(is_testenv_setup()) testenv.reset();
}

bool renderer_sdl::is_testenv_setup()
{
    return (testenv.get() ? true : false);
}

uintptr_t renderer_sdl::on_create_image(const uint32_t * data, int32_t width, int32_t height, int32_t channels)
{
    // PREPARE COLOR MASKS FOR IMAGE SURFACE:
    int bpp = 0;
    Uint32 rmask = 0, gmask = 0, bmask = 0, amask = 0;
    if (SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ABGR8888, &bpp, &rmask, &gmask, &bmask, &amask) == SDL_FALSE) {
        return reinterpret_cast<uintptr_t>(nullptr);
    }

    // CREATE AN SDL SURFACE & TEXTURE:
    SDL_Surface * surface;
    SDL_Texture * texture;
    // NOTE: SDL_CreateRGBSurfaceFrom() does not copy the buffer, but uses the existing buffer that we're managing.
    if ((surface = SDL_CreateRGBSurfaceFrom(
        (void*)data, width, height,
        bpp,                        // depth or bits per pixel (4 channels X 8 bits per channel)
        width * 4,                  // pitch (width X 4 channels)
        rmask, gmask, bmask, amask  // color masks
    )) == 0) {
        return reinterpret_cast<uintptr_t>(nullptr);
    }
    else {
        texture = SDL_CreateTextureFromSurface(reinterpret_cast<SDL_Renderer *>(m_sdl_renderer), surface);
        SDL_FreeSurface(surface);   // Clean-up the logo surface & buffer, it's no longer needed:
        surface = nullptr;

        if (texture == NULL) {
            reinterpret_cast<uintptr_t>(nullptr);
        }
    }

    return reinterpret_cast<uintptr_t>(texture);
}

void renderer_sdl::on_destroy_image(uintptr_t image)
{
    if (image) {
        SDL_DestroyTexture(reinterpret_cast<SDL_Texture *>(image));
    }
}

void renderer_sdl::on_draw_line(const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const color& clr) const
{
    if (m_sdl_renderer) {
        SDL_SetRenderDrawColor(
            reinterpret_cast<SDL_Renderer *>(m_sdl_renderer),
            clr.r,
            clr.g,
            clr.b,
            clr.a
        );

        SDL_RenderDrawLine(
            reinterpret_cast<SDL_Renderer *>(m_sdl_renderer), 
            x1, y1, x2, y2
        );
    }
}

void renderer_sdl::on_draw_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const color& clr, const uint32_t thickness) const
{
    if (m_sdl_renderer) {
        SDL_SetRenderDrawColor(
            reinterpret_cast<SDL_Renderer *>(m_sdl_renderer),
            clr.r,
            clr.g,
            clr.b,
            clr.a
        );

        SDL_Rect rect = { x, y, w, h };
        for (uint32_t i = 0; i < thickness; ++i) {
            SDL_RenderDrawRect(
                reinterpret_cast<SDL_Renderer *>(m_sdl_renderer), 
                &rect
            );
            rect.x += 1;
            rect.y += 1;
            rect.w -= 2;
            rect.h -= 2;
        }
    }
}

void renderer_sdl::on_fill_rect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const color& clr) const
{
    if (m_sdl_renderer) {
        SDL_SetRenderDrawColor(
            reinterpret_cast<SDL_Renderer *>(m_sdl_renderer),
            clr.r,
            clr.g,
            clr.b,
            clr.a
        );

        SDL_Rect rect = { x, y, w, h };
        SDL_RenderFillRect(
            reinterpret_cast<SDL_Renderer *>(m_sdl_renderer),
            &rect
        );
    }
}

void renderer_sdl::on_draw_image(
    uintptr_t source_img,
    const int32_t source_x, const int32_t source_y,
    const int32_t source_w, const int32_t source_h,
    const int32_t x, const int32_t y,
    const int32_t width, const int32_t height,
    const color& modulation,
    const rotation direction
) const
{
    if (m_sdl_renderer && source_img) {
        SDL_SetTextureColorMod(
            reinterpret_cast<SDL_Texture *>(source_img),
            modulation.r,
            modulation.g,
            modulation.b
        );
        SDL_SetTextureAlphaMod(
            reinterpret_cast<SDL_Texture *>(source_img),
            modulation.a
        );

        SDL_Rect srcrect = { source_x, source_y, source_w, source_h };
        SDL_Rect dstrect = { x, y, width, height };

        SDL_Point rotation_point = { 0, 0 };
        double angle = 0.f;
        switch (direction) {
        case rotation::right90degrees:
            dstrect.x += dstrect.h;
            angle = 90.f;
            break;
        case rotation::left90degrees:
            dstrect.y += dstrect.w;
            angle = -90.f;
            break;
        }

        SDL_RenderCopyEx(
            reinterpret_cast<SDL_Renderer *>(m_sdl_renderer),
            reinterpret_cast<SDL_Texture *>(source_img),
            &srcrect, &dstrect,
            angle, &rotation_point,
            SDL_RendererFlip::SDL_FLIP_NONE
        );
    }
}
