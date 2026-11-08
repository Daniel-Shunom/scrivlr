#include <clay.h>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <vector>

import scrivlr_ui;
import scrivlr_window;
import component_images;
import glad;
import component_workspace;

void layout(void* image) {
    CLAY({
        .layout =
            {
                .sizing = {CLAY_SIZING_FIXED(128), CLAY_SIZING_FIXED(128)},
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        .backgroundColor = {10, 20, 30, 255},
    }) {
        CLAY({
            .layout =
                {
                    .sizing = {CLAY_SIZING_FIXED(64), CLAY_SIZING_FIXED(64)},
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            .clip = {
                .horizontal = true,
                .vertical = true,
            },
        }) {
            CLAY({
                .layout =
                    {
                        .sizing = {CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32)},
                    },
                .clip = {
                    .horizontal = true,
                    .vertical = true,
                },
            }) {
                CLAY({
                    .layout =
                        {
                            .sizing = {CLAY_SIZING_FIXED(64), CLAY_SIZING_FIXED(64)},
                        },
                    .backgroundColor = {255, 0, 0, 255},
                }) {}
            }
            CLAY({
                .layout =
                    {
                        .sizing = {CLAY_SIZING_FIXED(64), CLAY_SIZING_FIXED(32)},
                    },
                .backgroundColor = {0, 255, 0, 255},
            }) {}
        }
        CLAY({
            .layout =
                {
                    .sizing = {CLAY_SIZING_FIXED(128), CLAY_SIZING_FIXED(32)},
                },
            .backgroundColor = {0, 0, 255, 255},
        }) {}
        CLAY({
            .layout =
                {
                    .sizing = {CLAY_SIZING_FIXED(16), CLAY_SIZING_FIXED(16)},
                },
            .image = {
                .imageData = image,
            },
        }) {}
        CLAY_TEXT(CLAY_STRING("Clay"),
            CLAY_TEXT_CONFIG({
                .textColor = {255, 255, 255, 255},
                .fontSize = 16,
            }));
    }
}

int main(int argc, char** argv) {
    try {
        Scrivlr::Window window(256, 256, "Clay renderer test", false);
        Scrivlr::UI::Renderer ui;
        auto image = Scrivlr::Components::Image::loadSvgFromMemory(
            "<svg width='2' height='2'><rect width='2' height='2' "
            "fill='yellow'/></svg>",
            2,
            2);
        if (!image)
            return 1;
        auto handle = ui.uploadImage(*image);
        GLuint texture = 0, framebuffer = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            256,
            256,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr);
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return 2;
        glViewport(3, 4, 20, 30);
        glActiveTexture(GL_TEXTURE5);
        ui.render({128, 128, 256, 256, -1, -1, false, 0, 0, 0.016f},
            &layout,
            handle.clayData());
        GLint viewport[4], active_texture = 0;
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
        if (viewport[0] != 3 || viewport[1] != 4 || viewport[2] != 20 ||
            viewport[3] != 30 || active_texture != GL_TEXTURE5)
            return 3;
        std::vector<unsigned char> pixels(256 * 256 * 4);
        glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        auto check = [&](int x, int y, int r, int g, int b) {
            const auto* p = pixels.data() + ((255 - y * 2) * 256 + x * 2) * 4;
            if (p[0] != r || p[1] != g || p[2] != b) {
                std::fprintf(stderr,
                    "Pixel %d,%d: %d,%d,%d; expected %d,%d,%d\n",
                    x,
                    y,
                    p[0],
                    p[1],
                    p[2],
                    r,
                    g,
                    b);
                return false;
            }
            return true;
        };
        if (!check(16, 16, 255, 0, 0) || !check(48, 16, 10, 20, 30) ||
            !check(16, 48, 0, 255, 0) || !check(80, 48, 10, 20, 30) ||
            !check(80, 80, 0, 0, 255) || !check(8, 104, 255, 255, 0))
            return 4;
        int text_pixels = 0;
        for (int y = 224; y < 256; ++y) {
            for (int x = 0; x < 80; ++x) {
                const auto* p = pixels.data() + ((255 - y) * 256 + x) * 4;
                if (p[0] > 100 && p[1] > 100 && p[2] > 100)
                    ++text_pixels;
            }
        }
        if (text_pixels < 10)
            return 5;
        if (glGetError() != GL_NO_ERROR)
            return 6;
        if (argc >= 2) {
            const int width = argc >= 3 ? std::atoi(argv[2]) : 1280;
            const int height = argc >= 4 ? std::atoi(argv[3]) : 800;
            if (width <= 0 || height <= 0)
                return 9;
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D,
                0,
                GL_RGBA8,
                width,
                height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                nullptr);
            Scrivlr::Components::Workspace workspace;
            workspace.data.width = width;
            workspace.data.height = height;
            if (argc >= 5)
                workspace.tabs.data.selected =
                    static_cast<std::size_t>(std::atoi(argv[4]));
            ui.render({width, height, width, height, -1, -1, false, 0, 0, 0.016f},
                &Scrivlr::Components::buildWorkspace,
                &workspace);
            std::vector<unsigned char> rgb(
                static_cast<std::size_t>(width) * height * 3);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());
            FILE* file = std::fopen(argv[1], "wb");
            if (!file)
                return 8;
            std::fprintf(file, "P6\n%d %d\n255\n", width, height);
            for (int y = height - 1; y >= 0; --y)
                std::fwrite(rgb.data() + static_cast<std::size_t>(y) * width * 3,
                    1,
                    static_cast<std::size_t>(width) * 3,
                    file);
            std::fclose(file);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &texture);
        std::puts("Clay rendering, text, image, nested clipping, DPI scaling, and "
                  "GL state "
                  "restoration passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 7;
    }
}
