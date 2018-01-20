#include "renderer_sdl.h"
#include "image.h"
#include <string>
#include <memory>
#include <SDL.h>

using namespace xeekworx::bitmapfonts;

renderer_sdl::renderer_sdl()
{

}

renderer_sdl::~renderer_sdl()
{

}

uint32_t renderer_sdl::set_foreground(uint32_t color)
{
    return renderer_sdl::set_foreground(color);
}

uint32_t renderer_sdl::set_background(uint32_t color)
{
    return renderer_base::set_background(color);
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

void renderer_sdl::close_testenv()
{
    testenv.reset();
}

bool renderer_sdl::is_testenv_setup()
{
    return (testenv.get() ? true : false);
}