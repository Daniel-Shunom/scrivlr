module;

#include "backend.h"

export module scrivlr_ui;
import component_images;

export namespace Scrivlr::UI {
using FrameInput = Frame;

class ImageHandle {
public:
    explicit ImageHandle(void* data) : data_(data) {}
    void* clayData() const { return data_; }

private:
    void* data_;
};

class Renderer {
public:
    Renderer() : backend_(createBackend()) {}
    ~Renderer() { destroyBackend(backend_); }
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    ImageHandle uploadImage(const Components::Image& image) {
        return ImageHandle(uploadBackendImage(
            backend_, image.width(), image.height(), image.pixels()));
    }

    void render(
        const FrameInput& frame, void (*build)(void*), void* user_data = nullptr) {
        renderBackend(backend_, frame, build, user_data);
    }

private:
    Backend* backend_;
};
} // namespace Scrivlr::UI
