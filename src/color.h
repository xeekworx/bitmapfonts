#pragma once
#include <stdint.h>
#include <cstdlib>      // std::strtoul
#include <string>
#include <sstream>      // std::stringstream
#include <ios>          // std::hex
#include <algorithm>    // std::transform

namespace xeekworx {
    namespace bitmapfonts {

        class color {
        public:
            // There is concern that this might not work the same on non x86 based processors
            // due to this code assuming little-endian.

            static constexpr uint32_t transparent = 0xFFFFFF00;
            static constexpr uint32_t white = 0xFFFFFFFF;
            static constexpr uint32_t black = 0x000000FF;

            uint8_t r, g, b, a;

            color() : r(0), g(0), b(0), a(0) {}
            color(const color& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
            color(uint32_t rgba_value) { *this = color::from_rgba(rgba_value); }
            color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
            color(const std::string& html) { *this = color::from_html(html); }

            /* rgba_value */ operator uint32_t() { return color::to_rgba(*this); }
            friend bool operator==(const color& c1, const color& c2) {
                return (
                    c1.r == c1.r &&
                    c1.g == c1.g &&
                    c1.b == c1.b &&
                    c1.a == c1.a);
            }
            friend bool operator!=(const color& c1, const color& c2) {
                return !(c1 == c2);
            }

            static color from_html(const std::string& html) {
                std::string formatted(html);
                formatted.erase(std::find(formatted.begin(), formatted.end(), '#'));
                if (formatted.length() == 8)
                    return color::from_rgba(std::strtoul(formatted.c_str(), 0, 16));
                else if (formatted.length() == 6)
                    return color::from_rgb(std::strtoul(formatted.c_str(), 0, 16));
                else
                    return color::black;
            }

            static color from_rgba(uint32_t v) {
                return color(
                    (v >> 24) & 0xFF,   // red
                    (v >> 16) & 0xFF,   // green
                    (v >> 8) & 0xFF,    // blue
                    v & 0xFF            // alpha
                );
            }

            static color from_rgba(uint32_t v, uint8_t alpha) {
                return color(
                    (v >> 24) & 0xFF,   // red
                    (v >> 16) & 0xFF,   // green
                    (v >> 8) & 0xFF,    // blue
                    alpha               // alpha
                );
            }

            static color from_rgb(uint32_t v, uint8_t alpha = 0xFF) {
                return color(
                    (v >> 16) & 0xFF,   // red
                    (v >> 8) & 0xFF,    // green
                    v & 0xFF,           // blue
                    alpha
                );
            }

            static color from_abgr(uint32_t v) {
                return color(
                    v & 0xFF,           // red
                    (v >> 8) & 0xFF,    // green
                    (v >> 16) & 0xFF,   // blue
                    (v >> 24) & 0xFF    // alpha
                );
            }

            static color from_abgr(uint32_t v, uint8_t alpha) {
                return color(
                    v & 0xFF,           // red
                    (v >> 8) & 0xFF,    // green
                    (v >> 16) & 0xFF,   // blue
                    alpha               // alpha
                );
            }

            static uint32_t to_rgba(const color& c) {
                return ((c.a) + (c.b << 8) + (c.g << 16) + (c.r << 24));
            }

            static uint32_t to_rgba(const color& c, uint8_t alpha) {
                return ((alpha)+(c.b << 8) + (c.g << 16) + (c.r << 24));
            }

            static uint32_t to_rgb(const color& c) {
                return ((c.b) + (c.g << 8) + (c.r << 16));
            }

            static uint32_t to_abgr(const color& c) {
                return ((c.r) + (c.g << 8) + (c.b << 16) + (c.a << 24));
            }

            static uint32_t to_abgr(const color& c, uint8_t alpha) {
                return ((c.r) + (c.g << 8) + (c.b << 16) + (alpha << 24));
            }

            static std::string to_html(const color& c, bool uppercase = false) {
                std::stringstream ss;
                ss << "#" << std::hex << to_rgb(c);
                if (uppercase) {
                    std::string s(ss.str());
                    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
                    return s;
                }
                else return ss.str();
            }

            static std::string to_string(const color& c, bool uppercase = false) {
                std::stringstream ss;
                ss << "#" << std::hex << to_rgba(c);
                if (uppercase) {
                    std::string s(ss.str());
                    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
                    return s;
                }
                else return ss.str();
            }

            static uint32_t blend(const uint32_t foreground, const uint32_t background) {
                return color::to_rgba(blend(color(foreground), color(background)));
            }

            static color blend(const color& foreground, const color& background) {
                if (background.a == 0) return foreground;
                else {
                    uint8_t final_alpha = foreground.a + background.a * (255 - foreground.a) / 255;
                    return color(
                        (foreground.r * foreground.a + background.r * background.a * (255 - foreground.a) / 255) / final_alpha,
                        (foreground.g * foreground.a + background.g * background.a * (255 - foreground.a) / 255) / final_alpha,
                        (foreground.b * foreground.a + background.b * background.a * (255 - foreground.a) / 255) / final_alpha,
                        final_alpha
                    );
                }
            }

        };

    }
}