module;

#include <GLFW/glfw3.h>
#include <exception>
#include <stdexcept>
#include <utility>

export module scrivlr_window;
import glad;
import component_images;

export namespace Scrivlr {

struct WindowInput {
    int width, height, framebuffer_width, framebuffer_height;
    float pointer_x, pointer_y;
    bool pointer_down;
    float scroll_x, scroll_y, delta_time;
    bool tab, shift, activate, left, right, escape;
};

class Window {
public:
    using RepaintCallback = void (*)(Window&, void*);

    Window(int width, int height, const char* title, bool visible = true) {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW initialization failed");
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
        handle_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!handle_) {
            glfwTerminate();
            throw std::runtime_error("Window creation failed");
        }
        glfwMakeContextCurrent(handle_);
        if (!gladLoadGLLoader(+[](const char* name) -> void* {
                return reinterpret_cast<void*>(glfwGetProcAddress(name));
            })) {
            glfwDestroyWindow(handle_);
            glfwTerminate();
            throw std::runtime_error("OpenGL function loading failed");
        }
        glfwSwapInterval(1);
        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize(handle_, &framebuffer_width, &framebuffer_height);
        glViewport(0, 0, framebuffer_width, framebuffer_height);

        glfwSetWindowUserPointer(handle_, this);
        glfwSetFramebufferSizeCallback(
            handle_, [](GLFWwindow* handle, int w, int h) {
                glViewport(0, 0, w, h);
                static_cast<Window*>(glfwGetWindowUserPointer(handle))->repaint();
            });
        glfwSetWindowRefreshCallback(handle_, [](GLFWwindow* handle) {
            static_cast<Window*>(glfwGetWindowUserPointer(handle))->repaint();
        });
        glfwSetScrollCallback(handle_, [](GLFWwindow* handle, double x, double y) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(handle));
            self->scroll_x_ += static_cast<float>(x);
            self->scroll_y_ += static_cast<float>(y);
        });
        last_frame_time_ = glfwGetTime();
    }

    ~Window() {
        glfwDestroyWindow(handle_);
        glfwTerminate();
    }
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    bool shouldClose() const { return glfwWindowShouldClose(handle_) != 0; }
    void pollEvents() {
        glfwPollEvents();
        if (callback_error_)
            std::rethrow_exception(std::exchange(callback_error_, {}));
    }
    void present() { glfwSwapBuffers(handle_); }
    void setRepaintCallback(RepaintCallback callback, void* data = nullptr) {
        repaint_ = callback;
        repaint_data_ = data;
    }
    WindowInput input() {
        WindowInput result{};
        glfwGetWindowSize(handle_, &result.width, &result.height);
        glfwGetFramebufferSize(
            handle_, &result.framebuffer_width, &result.framebuffer_height);
        double x, y;
        glfwGetCursorPos(handle_, &x, &y);
        result.pointer_x = static_cast<float>(x);
        result.pointer_y = static_cast<float>(y);
        result.pointer_down =
            glfwGetMouseButton(handle_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        const int keys[] = {
            GLFW_KEY_TAB,
            GLFW_KEY_ENTER,
            GLFW_KEY_SPACE,
            GLFW_KEY_LEFT,
            GLFW_KEY_RIGHT,
            GLFW_KEY_ESCAPE
        };
        bool pressed[6]{};
        for (int i = 0; i < 6; ++i) {
            const bool down = glfwGetKey(handle_, keys[i]) == GLFW_PRESS;
            pressed[i] = down && !keys_down_[i];
            keys_down_[i] = down;
        }
        result.tab = pressed[0];
        result.activate = pressed[1] || pressed[2];
        result.left = pressed[3];
        result.right = pressed[4];
        result.escape = pressed[5];
        result.shift = glfwGetKey(handle_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                       glfwGetKey(handle_, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        result.scroll_x = std::exchange(scroll_x_, 0.0f);
        result.scroll_y = std::exchange(scroll_y_, 0.0f);
        const double now = glfwGetTime();
        result.delta_time = static_cast<float>(now - last_frame_time_);
        last_frame_time_ = now;
        return result;
    }
    void setIcon(Components::Image& image) {
#ifndef __APPLE__
        GLFWimage view{image.width(), image.height(), image.pixels()};
        glfwSetWindowIcon(handle_, 1, &view);
#else
        (void)image;
#endif
    }

private:
    void repaint() noexcept {
        try {
            if (repaint_)
                repaint_(*this, repaint_data_);
        } catch (...) {
            callback_error_ = std::current_exception();
        }
    }
    GLFWwindow* handle_ = nullptr;
    RepaintCallback repaint_ = nullptr;
    void* repaint_data_ = nullptr;
    float scroll_x_ = 0;
    float scroll_y_ = 0;
    double last_frame_time_ = 0;
    bool keys_down_[6]{};
    std::exception_ptr callback_error_;
};

} // namespace Scrivlr
