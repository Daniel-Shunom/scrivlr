module;

#include <cstdio>

export module scrivlr_application;
import scrivlr_window;
import component_images;
import component_icons;
import glad;
import scrivlr_ui;
import component_workspace;

export namespace Scrivlr {

class Application {
public:
    explicit Application(bool run_smoke_test = false)
        : window(1280, 800, "scrivlr", !run_smoke_test), smoke_test(run_smoke_test) {
        window.setRepaintCallback(&renderFrame, this);
        auto icon =
            Components::Image::loadFromMemory(Components::Icons::WindowIconPng);
        if (icon) {
            window.setIcon(*icon);
        } else {
            std::fprintf(stderr, "Window icon: %s\n", icon.error().c_str());
        }
    }

    ~Application() { window.setRepaintCallback(nullptr); }

    int run() {
        while (!window.shouldClose()) {
            window.pollEvents();
            if (window.shouldClose()) {
                break;
            }
            renderFrame(window, this);
            if (smoke_test) {
                return glGetError() == GL_NO_ERROR ? 0 : 1;
            }
        }
        return 0;
    }

private:
    static void renderFrame(Window& window, void* data) {
        auto& self = *static_cast<Application*>(data);
        const auto input = window.input();
        self.workspace.data.width = input.width;
        self.workspace.data.height = input.height;
        self.workspace.context.beginFrame({
            .x = input.pointer_x,
            .y = input.pointer_y,
            .dt = input.delta_time,
            .wheel = input.scroll_y,
            .down = input.pointer_down,
            .tab = input.tab,
            .shift = input.shift,
            .activate = input.activate,
            .left = input.left,
            .right = input.right,
            .escape = input.escape,
        });
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const bool canvas_scroll =
            self.workspace.tabs.data.selected == 0 &&
            self.workspace.canvas.containsPoint(input.pointer_x, input.pointer_y);
        self.ui.render({input.width,
                           input.height,
                           input.framebuffer_width,
                           input.framebuffer_height,
                           input.pointer_x,
                           input.pointer_y,
                           input.pointer_down,
                           input.scroll_x,
                           canvas_scroll ? 0.0f : input.scroll_y,
                           input.delta_time},
            &Components::buildWorkspace,
            &self.workspace);
        self.workspace.context.endFrame();
        window.present();
    }

    Window window;
    UI::Renderer ui;
    Components::Workspace workspace;
    bool smoke_test;
};

} // namespace Scrivlr
