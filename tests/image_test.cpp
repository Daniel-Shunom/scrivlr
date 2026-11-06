#include <cstdio>
#include <span>
#include <utility>

import component_images;
import component_icons;

int main() {
    using Scrivlr::Components::Image;
    auto png = Image::loadFromMemory(Scrivlr::Components::Icons::WindowIconPng);
    if (!png || png->width() <= 0 || png->height() <= 0 ||
        png->byteSize() != static_cast<std::size_t>(png->width()) * png->height() * 4) {
        std::fprintf(stderr, "Embedded PNG decoding failed\n");
        return 1;
    }
    auto file = Image::loadFromFile("assets/icons/Scrivlr.png");
    if (!file || file->width() != png->width() || file->height() != png->height()) {
        return 2;
    }
    auto jpeg = Image::loadFromFile("tests/assets/sample.jpg");
    if (!jpeg || jpeg->width() != 2 || jpeg->height() != 3) {
        return 3;
    }
    if (Image::loadFromFile("assets/does-not-exist.png") ||
        Image::loadFromMemory(std::span<const unsigned char>{})) {
        return 4;
    }
    const unsigned char invalid[] = {1, 2, 3, 4};
    if (Image::loadFromMemory(invalid)) {
        return 5;
    }
    auto* originalPixels = png->pixels();
    Image moved = std::move(*png);
    if (moved.pixels() != originalPixels || png->pixels() || png->byteSize() != 0) {
        return 6;
    }
    *jpeg = std::move(moved);
    if (jpeg->pixels() != originalPixels || moved.pixels() || moved.byteSize() != 0) {
        return 7;
    }
    auto svg = Image::loadSvgFromFile("tests/assets/sample.svg", 32, 32);
    if (!svg || svg->width() != 32 || svg->height() != 32) {
        return 8;
    }
    const auto* center = svg->pixels() + (16 * 32 + 16) * 4;
    if (center[0] != 255 || center[1] != 0 || center[2] != 0 || center[3] != 255 ||
        svg->pixels()[3] != 0) {
        return 9;
    }
    auto memorySvg = Image::loadSvgFromMemory(
        "<svg width='8' height='8'><rect width='8' height='8' fill='blue'/></svg>", 8, 8);
    if (!memorySvg || memorySvg->pixels()[(4 * 8 + 4) * 4 + 2] != 255) {
        return 10;
    }
    if (Image::loadSvgFromMemory("garbage", 32, 32) ||
        Image::loadSvgFromMemory("", 32, 32) ||
        Image::loadSvgFromMemory("<svg/>", 0, 32) ||
        Image::loadSvgFromMemory("<svg/>", 8193, 32) ||
        Image::loadSvgFromFile("missing.svg", 32, 32)) {
        return 11;
    }
    return 0;
}
