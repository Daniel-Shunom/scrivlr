#pragma once

namespace Scrivlr::UI {
struct Frame {
    int width = 0;
    int height = 0;
    int framebuffer_width = 0;
    int framebuffer_height = 0;
    float pointer_x = 0;
    float pointer_y = 0;
    bool pointer_down = false;
    float scroll_x = 0;
    float scroll_y = 0;
    float delta_time = 0;
};

struct Backend;
Backend* createBackend();
void destroyBackend(Backend*) noexcept;
void* uploadBackendImage(Backend*, int width, int height, const unsigned char* pixels);
void renderBackend(Backend*, const Frame&, void (*build)(void*), void*);
}
