#include "backend.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <glad/glad.h>
#include <memory>
#include <stb_truetype.h>
#include <stdexcept>
#include <string>
#include <vector>

#define CLAY_IMPLEMENTATION
#include <clay.h>
#define GLSL_VERSION "#version 330 core"
#define CLAY_RENDERER_GLES3_IMPLEMENTATION
#include <clay_renderer_gles3.h>

namespace {
constexpr unsigned char font_bytes[] = {
#embed "../../assets/fonts/NotoSans-Regular.ttf"
};

struct GLState {
    GLint program, vao, buffer, active_texture, unpack_alignment;
    GLint textures[4], samplers[4], scissor_box[4], viewport[4];
    GLint src_rgb, dst_rgb, src_alpha, dst_alpha, equation_rgb, equation_alpha;
    GLboolean blend, depth, cull, scissor;

    GLState() {
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack_alignment);
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor_box);
        glGetIntegerv(GL_BLEND_SRC_RGB, &src_rgb);
        glGetIntegerv(GL_BLEND_DST_RGB, &dst_rgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &src_alpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &dst_alpha);
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &equation_rgb);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &equation_alpha);
        blend = glIsEnabled(GL_BLEND);
        depth = glIsEnabled(GL_DEPTH_TEST);
        cull = glIsEnabled(GL_CULL_FACE);
        scissor = glIsEnabled(GL_SCISSOR_TEST);
        for (GLuint i = 0; i < 4; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &textures[i]);
            glGetIntegerv(GL_SAMPLER_BINDING, &samplers[i]);
            glBindSampler(i, 0);
        }
    }

    ~GLState() {
        for (GLuint i = 0; i < 4; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glBindSampler(i, samplers[i]);
        }
        glActiveTexture(active_texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_alignment);
        glUseProgram(program);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(scissor_box[0], scissor_box[1], scissor_box[2], scissor_box[3]);
        glBlendEquationSeparate(equation_rgb, equation_alpha);
        glBlendFuncSeparate(src_rgb, dst_rgb, src_alpha, dst_alpha);
        setEnabled(GL_BLEND, blend);
        setEnabled(GL_DEPTH_TEST, depth);
        setEnabled(GL_CULL_FACE, cull);
        setEnabled(GL_SCISSOR_TEST, scissor);
    }

    static void setEnabled(GLenum capability, GLboolean enabled) {
        if (enabled)
            glEnable(capability);
        else
            glDisable(capability);
    }
};
} // namespace

namespace Scrivlr::UI {
struct Backend {
    Gles3_Renderer renderer{};
    std::vector<unsigned char> arena;
    Clay_Context* context = nullptr;
    stbtt_bakedchar glyphs[96]{};
    float ascent = 0;
    float line_height = 0;
    bool clay_error = false;
    Gles3_ImageConfig images[MAX_IMAGES]{};
    int image_count = 0;

    ~Backend() {
        glDeleteTextures(MAX_FONTS, renderer.fontTextures);
        glDeleteTextures(MAX_IMAGES, renderer.imageTextures);
        glDeleteBuffers(1, &renderer.quadVBO);
        glDeleteBuffers(1, &renderer.quadInstanceVBO);
        glDeleteBuffers(1, &renderer.textVBO);
        glDeleteVertexArrays(1, &renderer.quadVAO);
        glDeleteVertexArrays(1, &renderer.textVAO);
        glDeleteProgram(renderer.quadShaderId);
        glDeleteProgram(renderer.textShader);
        std::free(renderer.quadInstanceArray.instData);
        std::free(renderer.glyphVtxArray.instData);
        if (Clay_GetCurrentContext() == context)
            Clay_SetCurrentContext(nullptr);
    }

    void initialize() {
        Gles3_Initialize(&renderer, 8192);
        if (!renderer.quadInstanceArray.instData ||
            !renderer.glyphVtxArray.instData) {
            throw std::runtime_error("Cannot allocate Clay rendering buffers");
        }
        for (GLuint program : {renderer.quadShaderId, renderer.textShader}) {
            GLint linked = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &linked);
            if (!linked) {
                char log[2048]{};
                glGetProgramInfoLog(program, sizeof(log), nullptr, log);
                throw std::runtime_error(
                    std::string("Clay shader link failed: ") + log);
            }
        }
        std::vector<unsigned char> atlas(1024 * 1024);
        if (stbtt_BakeFontBitmap(
                font_bytes, 0, 48.0f, atlas.data(), 1024, 1024, 32, 96, glyphs) <=
            0) {
            throw std::runtime_error("Cannot bake UI font atlas");
        }
        stbtt_fontinfo font{};
        if (!stbtt_InitFont(&font, font_bytes, 0))
            throw std::runtime_error("Invalid UI font");
        int a, d, gap;
        stbtt_GetFontVMetrics(&font, &a, &d, &gap);
        const float scale = stbtt_ScaleForPixelHeight(&font, 48.0f);
        ascent = a * scale;
        line_height = (a - d) * scale;
        glGenTextures(1, &renderer.fontTextures[0]);
        glBindTexture(GL_TEXTURE_2D, renderer.fontTextures[0]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_R8,
            1024,
            1024,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            atlas.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        renderer.renderTextFunction = &renderText;
        arena.resize(Clay_MinMemorySize());
        context = Clay_Initialize(
            Clay_CreateArenaWithCapacityAndMemory(arena.size(), arena.data()),
            {1, 1},
            {
                .errorHandlerFunction =
                    [](Clay_ErrorData error) {
                        static_cast<Backend*>(error.userData)->clay_error = true;
                        std::fprintf(stderr,
                            "Clay: %.*s\n",
                            error.errorText.length,
                            error.errorText.chars);
                    },
                .userData = this,
            });
        if (!context || clay_error)
            throw std::runtime_error("Cannot initialize Clay");
        Clay_SetMeasureTextFunction(&measureText, this);
    }

    static int glyphIndex(unsigned char c) {
        return (c >= 32 && c < 128 ? c : '?') - 32;
    }

    static Clay_Dimensions measureText(
        Clay_StringSlice text, Clay_TextElementConfig* config, void* data) {
        auto& self = *static_cast<Backend*>(data);
        const float scale = config->fontSize / 48.0f;
        float width = 0;
        for (int i = 0; i < text.length; ++i) {
            width +=
                self.glyphs[glyphIndex(static_cast<unsigned char>(text.chars[i]))]
                    .xadvance *
                scale;
            if (i + 1 < text.length)
                width += config->letterSpacing;
        }
        return {width, self.line_height * scale};
    }

    static void renderText(
        Clay_RenderCommand* cmd, Gles3_GlyphVtxArray* output, void* data) {
        auto& self = *static_cast<Backend*>(data);
        const auto& text = cmd->renderData.text;
        if (text.fontId != 0)
            throw std::runtime_error("Only UI font ID 0 is registered");
        const float scale = text.fontSize / 48.0f;
        float x = cmd->boundingBox.x;
        const float y = cmd->boundingBox.y + self.ascent * scale;
        for (int i = 0; i < text.stringContents.length; ++i) {
            if (output->count >= output->capacity)
                throw std::runtime_error("UI glyph buffer exhausted");
            const auto& g = self.glyphs[glyphIndex(
                static_cast<unsigned char>(text.stringContents.chars[i]))];
            float x0 = x + g.xoff * scale, y0 = y + g.yoff * scale;
            float x1 = x0 + (g.x1 - g.x0) * scale, y1 = y0 + (g.y1 - g.y0) * scale;
            float u0 = g.x0 / 1024.0f, v0 = g.y0 / 1024.0f;
            float u1 = g.x1 / 1024.0f, v1 = g.y1 / 1024.0f;
            float r = text.textColor.r / 255.0f, green = text.textColor.g / 255.0f;
            float b = text.textColor.b / 255.0f, a = text.textColor.a / 255.0f;
            GlyphVtx* v = output->instData + output->count * 6;
            v[0] = {x0, y0, u0, v0, r, green, b, a, 0, {}};
            v[1] = {x1, y0, u1, v0, r, green, b, a, 0, {}};
            v[2] = {x0, y1, u0, v1, r, green, b, a, 0, {}};
            v[3] = v[2];
            v[4] = v[1];
            v[5] = {x1, y1, u1, v1, r, green, b, a, 0, {}};
            ++output->count;
            x += g.xadvance * scale + text.letterSpacing;
        }
    }
};

Backend* createBackend() {
    GLState state;
    auto result = std::make_unique<Backend>();
    Clay_Context* previous = Clay_GetCurrentContext();
    try {
        result->initialize();
    } catch (...) {
        Clay_SetCurrentContext(previous);
        throw;
    }
    Clay_SetCurrentContext(previous);
    return result.release();
}

void destroyBackend(Backend* backend) noexcept { delete backend; }

void* uploadBackendImage(
    Backend* backend, int width, int height, const unsigned char* pixels) {
    if (!pixels || width <= 0 || height <= 0)
        throw std::runtime_error("Cannot upload an empty UI image");
    if (backend->image_count >= MAX_IMAGES)
        throw std::runtime_error("Clay backend supports four image textures");
    GLState state;
    const int slot = backend->image_count++;
    glGenTextures(1, &backend->renderer.imageTextures[slot]);
    glBindTexture(GL_TEXTURE_2D, backend->renderer.imageTextures[slot]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    backend->images[slot] = {slot, 0, 0, 1, 1};
    return &backend->images[slot];
}

void renderBackend(
    Backend* backend, const Frame& frame, void (*build)(void*), void* data) {
    if (!build || frame.width <= 0 || frame.height <= 0 ||
        frame.framebuffer_width <= 0 || frame.framebuffer_height <= 0)
        return;
    GLState state;
    Clay_Context* previous = Clay_GetCurrentContext();
    struct RestoreContext {
        Clay_Context* context;
        ~RestoreContext() { Clay_SetCurrentContext(context); }
    } restore{previous};
    Clay_SetCurrentContext(backend->context);
    backend->clay_error = false;
    Clay_SetLayoutDimensions(
        {static_cast<float>(frame.width), static_cast<float>(frame.height)});
    Clay_SetPointerState({frame.pointer_x, frame.pointer_y}, frame.pointer_down);
    Clay_UpdateScrollContainers(
        false, {frame.scroll_x, frame.scroll_y}, frame.delta_time);
    Clay_BeginLayout();
    build(data);
    auto commands = Clay_EndLayout();
    if (backend->clay_error)
        throw std::runtime_error("Clay layout failed");
    glViewport(0, 0, frame.framebuffer_width, frame.framebuffer_height);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(
        GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    struct Clip {
        int x, y, w, h;
    };
    std::vector<Clip> clips;
    const float sx = static_cast<float>(frame.framebuffer_width) / frame.width;
    const float sy = static_cast<float>(frame.framebuffer_height) / frame.height;
    for (int i = 0; i < commands.length; ++i) {
        auto& command = commands.internalArray[i];
        if (command.commandType == CLAY_RENDER_COMMAND_TYPE_SCISSOR_START) {
            const auto& box = command.boundingBox;
            int x0 = static_cast<int>(std::floor(box.x * sx));
            int y0 = static_cast<int>(
                std::floor((frame.height - box.y - box.height) * sy));
            int x1 = static_cast<int>(std::ceil((box.x + box.width) * sx));
            int y1 = static_cast<int>(std::ceil((frame.height - box.y) * sy));
            Clip parent =
                clips.empty()
                    ? Clip{0, 0, frame.framebuffer_width, frame.framebuffer_height}
                    : clips.back();
            x0 = std::max(x0, parent.x);
            y0 = std::max(y0, parent.y);
            x1 = std::min(x1, parent.x + parent.w);
            y1 = std::min(y1, parent.y + parent.h);
            clips.push_back({x0, y0, std::max(0, x1 - x0), std::max(0, y1 - y0)});
        } else if (command.commandType == CLAY_RENDER_COMMAND_TYPE_SCISSOR_END) {
            if (!clips.empty())
                clips.pop_back();
        } else if (command.commandType != CLAY_RENDER_COMMAND_TYPE_NONE) {
            if (command.commandType == CLAY_RENDER_COMMAND_TYPE_CUSTOM) {
                throw std::runtime_error(
                    "Clay custom commands need a custom-draw adapter");
            }
            if (command.commandType == CLAY_RENDER_COMMAND_TYPE_IMAGE) {
                bool registered = false;
                for (int slot = 0; slot < backend->image_count; ++slot) {
                    registered |=
                        command.renderData.image.imageData == &backend->images[slot];
                }
                if (!registered)
                    throw std::runtime_error(
                        "UI image was not uploaded by this renderer");
                const auto& tint = command.renderData.image.backgroundColor;
                if (tint.r == 0 && tint.g == 0 && tint.b == 0 && tint.a == 0) {
                    command.renderData.image.backgroundColor = {255, 255, 255, 255};
                }
            }
            Gles3_Render(&backend->renderer, {1, 1, &command}, backend);
        }
        if (clips.empty()) {
            glDisable(GL_SCISSOR_TEST);
        } else {
            glEnable(GL_SCISSOR_TEST);
            const auto& clip = clips.back();
            glScissor(clip.x, clip.y, clip.w, clip.h);
        }
    }
}
} // namespace Scrivlr::UI
