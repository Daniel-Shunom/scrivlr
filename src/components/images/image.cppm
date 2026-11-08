module;

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <expected>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <nanosvg.h>
#include <nanosvgrast.h>
#include <span>
#include <stb_image.h>
#include <string>
#include <string_view>
#include <utility>

export module component_images;

export namespace Scrivlr::Components {

class Image {
public:
    static std::expected<Image, std::string> loadFromFile(const std::string& path);
    static std::expected<Image, std::string> loadFromMemory(
        std::span<const unsigned char> encoded_bytes);

    static std::expected<Image, std::string> loadSvgFromMemory(
        std::string_view svg, int width, int height);

    static std::expected<Image, std::string> loadSvgFromFile(
        const std::string& path, int width, int height);

    Image(const Image&) = delete;

    Image& operator=(const Image&) = delete;

    Image(Image&& other) noexcept
        : pixels_(std::move(other.pixels_)), width_(std::exchange(other.width_, 0)),
          height_(std::exchange(other.height_, 0)) {}

    Image& operator=(Image&& other) noexcept {
        if (this != &other) {
            pixels_ = std::move(other.pixels_);
            width_ = std::exchange(other.width_, 0);
            height_ = std::exchange(other.height_, 0);
        }

        return *this;
    }

    int width() const noexcept { return width_; }

    int height() const noexcept { return height_; }

    static constexpr int channels() noexcept { return 4; }

    std::size_t byteSize() const noexcept {
        return static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_) *
               channels();
    }

    unsigned char* pixels() noexcept { return pixels_.get(); }

    const unsigned char* pixels() const noexcept { return pixels_.get(); }

private:
    struct PixelDeleter {
        void operator()(unsigned char* pixels) const noexcept {
            stbi_image_free(pixels);
        }
    };

    Image(unsigned char* pixels, int width, int height)
        : pixels_(pixels), width_(width), height_(height) {}

    std::unique_ptr<unsigned char, PixelDeleter> pixels_;

    int width_;

    int height_;
};

std::expected<Image, std::string> Image::loadFromFile(const std::string& path) {
    int width = 0;

    int height = 0;

    auto* pixels = stbi_load(path.c_str(), &width, &height, nullptr, STBI_rgb_alpha);

    if (!pixels) {
        const char* reason = stbi_failure_reason();
        return std::unexpected(std::string("Cannot load image '") + path +
                               "': " + (reason ? reason : "unknown decoder error"));
    }

    return Image(pixels, width, height);
}

std::expected<Image, std::string> Image::loadFromMemory(
    std::span<const unsigned char> encoded_bytes) {
    if (encoded_bytes.empty() ||
        encoded_bytes.size() >
            static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return std::unexpected(std::string("Encoded image is empty or too large"));
    }

    int width = 0;

    int height = 0;

    auto* pixels = stbi_load_from_memory(encoded_bytes.data(),
        static_cast<int>(encoded_bytes.size()),
        &width,
        &height,
        nullptr,
        STBI_rgb_alpha);
    if (!pixels) {
        const char* reason = stbi_failure_reason();
        return std::unexpected(
            std::string(reason ? reason : "Unknown decoder error"));
    }

    return Image(pixels, width, height);
}

std::expected<Image, std::string> Image::loadSvgFromFile(

    const std::string& path, int width, int height) {

    std::ifstream file(path, std::ios::binary);

    if (!file) {
        return std::unexpected(std::string("Cannot open SVG: ") + path);
    }
    const std::string SOURCE((std::istreambuf_iterator<char>(file)), {});

    if (file.bad()) {
        return std::unexpected(std::string("Cannot read SVG: ") + path);
    }

    return loadSvgFromMemory(SOURCE, width, height);
}

std::expected<Image, std::string> Image::loadSvgFromMemory(
    std::string_view svg, int width, int height) {

    constexpr int MAX_DIMENSION = 8192;

    if (svg.empty() || width <= 0 || height <= 0 || width > MAX_DIMENSION ||
        height > MAX_DIMENSION) {
        return std::unexpected(
            std::string("SVG needs source data and dimensions between 1 and 8192"));
    }

    std::string source(svg);

    std::unique_ptr<NSVGimage, decltype(&nsvgDelete)> document(
        nsvgParse(source.data(), "px", 96.0f), &nsvgDelete);

    if (!document || !document->shapes || !std::isfinite(document->width) ||
        !std::isfinite(document->height) || document->width <= 0 ||
        document->height <= 0) {
        return std::unexpected(
            std::string("SVG has no drawable shapes or valid dimensions"));
    }

    std::unique_ptr<NSVGrasterizer, decltype(&nsvgDeleteRasterizer)> rasterizer(
        nsvgCreateRasterizer(), &nsvgDeleteRasterizer);

    if (!rasterizer) {
        return std::unexpected(std::string("Cannot allocate SVG rasterizer"));
    }

    auto* pixels = static_cast<unsigned char*>(std::calloc(
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height), 4));

    if (!pixels) {
        return std::unexpected(std::string("Cannot allocate SVG pixels"));
    }

    Image image(pixels, width, height);

    const float SCALE = std::min(static_cast<float>(width) / document->width,
        static_cast<float>(height) / document->height);
    if (!std::isfinite(SCALE) || SCALE <= 0) {
        return std::unexpected(
            std::string("SVG dimensions cannot be scaled safely"));
    }

    const float X = (static_cast<float>(width) - document->width * SCALE) * 0.5f;

    const float Y = (static_cast<float>(height) - document->height * SCALE) * 0.5f;

    nsvgRasterize(rasterizer.get(),
        document.get(),
        X,
        Y,
        SCALE,
        image.pixels(),
        width,
        height,
        width * 4);

    return image;
}

} // namespace Scrivlr::Components
