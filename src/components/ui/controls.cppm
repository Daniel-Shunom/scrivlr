module;
#include <clay.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>
export module scrivlr_controls;
export import scrivlr_widget_context;
export import scrivlr_animation;
import components;

export namespace Scrivlr::Components {
enum class ButtonVariant { Primary, Secondary, Ghost, Danger };

class Button : public Component {
public:
    struct Data {
        std::string id, label;
        ButtonVariant variant = ButtonVariant::Primary;
        bool enabled = true, visible = true, loading = false;
        std::function<void()> onClick{};
    } data;

    Button(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}

    void renderComponent() override {
        if (!data.visible) return;

        auto& t = context_.theme;

        const auto state = context_.interact(UI::elementId(data.id), data.enabled && !data.loading);

        hover_.target(state.hovered ? 1.0f : 0.0f);

        const float hover = hover_.update(context_.input.dt, context_.reduced_motion);

        auto color = data.variant == ButtonVariant::Primary ? t.accent :
            data.variant == ButtonVariant::Danger ? t.danger :
            data.variant == ButtonVariant::Ghost ? t.surface : t.subtle;

        auto foreground = data.variant == ButtonVariant::Primary || data.variant == ButtonVariant::Danger
            ? t.surface : t.text;

        if (!data.enabled || data.loading) { color = t.disabled; foreground = t.muted; }
        else color = UI::mix(color, data.variant == ButtonVariant::Primary ? t.accent_hover : t.border,
            state.held ? 0.45f : hover * 0.15f);

        CLAY({.id = UI::elementId(data.id), .layout = {.sizing = {CLAY_SIZING_FIT(0), CLAY_SIZING_FIXED(36)},
            .padding = {14,14,0,0}, .childGap = 8, .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}},
            .backgroundColor = color, .cornerRadius = CLAY_CORNER_RADIUS(t.radius),
            .border = {.color = t.accent, .width = UI::outline(static_cast<uint16_t>(state.focused ? 2 : 0))}}) {
            context_.text(data.loading ? loading_label_ : data.label, foreground, t.font_size);
            renderChildren();
        }

        if (state.clicked && data.onClick) {
            data.onClick();
        }
    }
private:
    UI::Context& context_;
    UI::Animation hover_;
    std::string loading_label_ = "Working...";
};

class Label : public Component {
public:
    struct Data { std::string text; uint16_t size = 16; bool muted = false, visible = true; } data;
    Label(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}
    void renderComponent() override {
        if (data.visible) context_.text(data.text, data.muted ? context_.theme.muted : context_.theme.text, data.size);
    }
private: UI::Context& context_;
};

class Checkbox : public Component {
public:
    struct Data {
        std::string id, label;
        bool checked = false, enabled = true, visible = true;
        std::function<void(bool)> onChange{};
    } data;
    Checkbox(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}
    void renderComponent() override {
        if (!data.visible) return;
        const auto s = context_.interact(UI::elementId(data.id), data.enabled);
        if (s.clicked) { data.checked = !data.checked; if (data.onChange) data.onChange(data.checked); }
        auto& t = context_.theme;
        CLAY({.id = UI::elementId(data.id), .layout = {.sizing = {CLAY_SIZING_FIT(0), CLAY_SIZING_FIXED(32)},
            .childGap = 10, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}},
            .cornerRadius = CLAY_CORNER_RADIUS(4),
            .border = {.color = t.accent, .width = UI::outline(static_cast<uint16_t>(s.focused ? 2 : 0))}}) {
            CLAY({.layout = {.sizing = {CLAY_SIZING_FIXED(18), CLAY_SIZING_FIXED(18)},
                .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}},
                .backgroundColor = !data.enabled ? t.disabled : data.checked ? t.accent : t.surface,
                .cornerRadius = CLAY_CORNER_RADIUS(4), .border = {.color = t.border, .width = UI::outline(1)}}) {
                if (data.checked) {
                    CLAY({.layout = {.sizing = {CLAY_SIZING_FIXED(8), CLAY_SIZING_FIXED(8)}},
                        .backgroundColor = t.surface, .cornerRadius = CLAY_CORNER_RADIUS(1)}) {}
                }
            }
            context_.text(data.label, data.enabled ? t.text : t.muted, t.font_size);
        }
    }
private: UI::Context& context_;
};

class Switch : public Component {
public:
    struct Data {
        std::string id, label;
        bool checked = false, enabled = true, visible = true;
        std::function<void(bool)> onChange{};
    } data;
    Switch(UI::Context& context, Data value) : data(std::move(value)), context_(context), position_(data.checked ? 1.0f : 0.0f) {}
    void renderComponent() override {
        if (!data.visible) return;
        const auto s = context_.interact(UI::elementId(data.id), data.enabled);
        if (s.clicked) { data.checked = !data.checked; if (data.onChange) data.onChange(data.checked); }
        position_.target(data.checked ? 1.0f : 0.0f);
        const float p = position_.update(context_.input.dt, context_.reduced_motion || (s.focused && context_.input.activate));
        auto& t = context_.theme;
        CLAY({.id = UI::elementId(data.id), .layout = {.sizing = {CLAY_SIZING_FIT(0), CLAY_SIZING_FIXED(32)},
            .childGap = 10, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}},
            .border = {.color = t.accent, .width = UI::outline(static_cast<uint16_t>(s.focused ? 2 : 0))}}) {
            CLAY({.layout = {.sizing = {CLAY_SIZING_FIXED(38), CLAY_SIZING_FIXED(22)}},
                .backgroundColor = !data.enabled ? t.disabled : UI::mix(t.muted,t.accent,p),
                .cornerRadius = CLAY_CORNER_RADIUS(11)}) {
                CLAY({.layout = {.sizing = {CLAY_SIZING_FIXED(16), CLAY_SIZING_FIXED(16)}},
                    .backgroundColor = t.surface, .cornerRadius = CLAY_CORNER_RADIUS(8),
                    .floating = {.offset = {3 + 16*p,3}, .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                        .attachTo = CLAY_ATTACH_TO_PARENT}}) {}
            }
            context_.text(data.label, data.enabled ? t.text : t.muted, t.font_size);
        }
    }
private: UI::Context& context_; UI::Animation position_;
};

class Slider : public Component {
public:
    struct Data {
        std::string id, label;
        float value = 0.5f, minimum = 0, maximum = 1, step = 0.01f, width = 220;
        bool enabled = true, visible = true;
        std::function<void(float)> onChange{};
    } data;
    Slider(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}
    void renderComponent() override {
        if (!data.visible) return;
        const float span = std::max(0.0f, data.maximum-data.minimum);
        data.value = std::clamp(data.value, data.minimum, data.minimum+span);
        const auto id = UI::elementId(data.id);
        const auto s = context_.interact(id, data.enabled && span > 0);
        float next = data.value;
        const auto box = Clay_GetElementData(id);
        if (s.held && box.found) next = data.minimum + span * std::clamp(
            (context_.input.x-box.boundingBox.x-8)/std::max(1.0f, box.boundingBox.width-16),0.0f,1.0f);
        const float step = data.step > 0 ? data.step : span / 100;
        if (s.focused) { if (context_.input.left) next -= step; if (context_.input.right) next += step; }
        if (next != data.value) {
            if (data.step > 0) next = data.minimum + std::round((next-data.minimum)/data.step)*data.step;
            next = std::clamp(next, data.minimum, data.minimum+span);
            if (next != data.value) { data.value = next; if (data.onChange) data.onChange(next); }
        }
        const float ratio = span > 0 ? (data.value-data.minimum)/span : 0;
        auto& t = context_.theme;
        UI::column([&] {
            context_.text(data.label, data.enabled ? t.text : t.muted, t.font_size);
            CLAY({.id = id, .layout = {.sizing = {CLAY_SIZING_FIXED(std::max(32.0f,data.width)), CLAY_SIZING_FIXED(28)},
                .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}}, .cornerRadius = CLAY_CORNER_RADIUS(4),
                .border = {.color = t.accent, .width = UI::outline(static_cast<uint16_t>(s.focused ? 2 : 0))}}) {
                CLAY({.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(4)}},
                    .backgroundColor = t.border, .cornerRadius = CLAY_CORNER_RADIUS(2)}) {}
                CLAY({.layout = {.sizing = {CLAY_SIZING_FIXED(16), CLAY_SIZING_FIXED(16)}},
                    .backgroundColor = data.enabled ? t.accent : t.muted, .cornerRadius = CLAY_CORNER_RADIUS(8),
                    .floating = {.offset = {ratio*(std::max(32.0f,data.width)-16),6},
                        .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH, .attachTo = CLAY_ATTACH_TO_PARENT}}) {}
            }
        }, 4);
    }
private: UI::Context& context_;
};

class Progress : public Component {
public:
    struct Data { std::string id; float value = 0, width = 220; bool visible = true; } data;
    Progress(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}
    void renderComponent() override {
        if (!data.visible) return;
        auto& t = context_.theme;
        const float width = std::max(1.0f,data.width), value = std::clamp(data.value,0.0f,1.0f);
        CLAY({.id = UI::elementId(data.id), .layout = {.sizing = {CLAY_SIZING_FIXED(width), CLAY_SIZING_FIXED(6)}},
            .backgroundColor = t.border, .cornerRadius = CLAY_CORNER_RADIUS(3)}) {
            if (value > 0) {
                CLAY({.layout = {.sizing = {CLAY_SIZING_FIXED(width*value), CLAY_SIZING_FIXED(6)}},
                    .backgroundColor = t.accent, .cornerRadius = CLAY_CORNER_RADIUS(3)}) {}
            }
        }
    }
private: UI::Context& context_;
};

class Tabs : public Component {
public:
    struct Data {
        std::string id; std::vector<std::string> labels; std::size_t selected = 0;
        bool enabled = true, visible = true;
        std::function<void(std::size_t)> onChange{};
    } data;
    Tabs(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}
    void renderComponent() override {
        if (!data.visible || data.labels.empty()) return;
        data.selected = std::min(data.selected,data.labels.size()-1);
        ids_.resize(data.labels.size());
        auto& t = context_.theme;
        CLAY({.id = UI::elementId(data.id), .layout = {.padding = CLAY_PADDING_ALL(4), .childGap = 4},
            .backgroundColor = t.subtle, .cornerRadius = CLAY_CORNER_RADIUS(t.radius)}) {
            for (std::size_t i = 0; i < data.labels.size(); ++i) {
                ids_[i] = data.id + "/" + std::to_string(i);
                const auto s = context_.interact(UI::elementId(ids_[i]),data.enabled);
                if (s.clicked && data.selected != i) { data.selected = i; if (data.onChange) data.onChange(i); }
                CLAY({.id = UI::elementId(ids_[i]), .layout = {.sizing = {CLAY_SIZING_FIT(0),CLAY_SIZING_FIXED(30)},
                    .padding = {12,12,0,0}, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}},
                    .backgroundColor = data.selected == i ? t.surface : t.subtle,
                    .cornerRadius = CLAY_CORNER_RADIUS(4),
                    .border = {.color = t.accent, .width = UI::outline(static_cast<uint16_t>(s.focused ? 2 : 0))}}) {
                    context_.text(data.labels[i],data.enabled ? t.text : t.muted,t.font_size);
                }
            }
        }
    }
private: UI::Context& context_; std::vector<std::string> ids_;
};

class Panel : public Component {
public:
    struct Data {
        std::string id;
        bool visible = true, horizontal = false, scroll = false;
        float height = 0;
    } data;
    Panel(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}
    void renderComponent() override {
        if (!data.visible) return;
        auto& t = context_.theme;
        CLAY({.id = UI::elementId(data.id), .layout = {.sizing = {CLAY_SIZING_GROW(0),
            data.height > 0 ? CLAY_SIZING_FIXED(data.height) : CLAY_SIZING_FIT(0)},
            .padding = CLAY_PADDING_ALL(t.padding), .childGap = t.gap,
            .layoutDirection = data.horizontal ? CLAY_LEFT_TO_RIGHT : CLAY_TOP_TO_BOTTOM},
            .backgroundColor = t.surface, .cornerRadius = CLAY_CORNER_RADIUS(t.radius),
            .clip = {.vertical = data.scroll, .childOffset = data.scroll ? Clay_GetScrollOffset() : Clay_Vector2{}},
            .border = {.color = t.border, .width = UI::outline(1)}}) {
            renderChildren();
        }
    }
private: UI::Context& context_;
};

class Separator : public Component {
public:
    struct Data { bool vertical = false, visible = true; float length = 0; } data;
    explicit Separator(UI::Context& context) : context_(context) {}
    Separator(UI::Context& context, Data value) : data(value), context_(context) {}
    void renderComponent() override {
        if (!data.visible) return;
        auto length = data.length > 0 ? CLAY_SIZING_FIXED(data.length) : CLAY_SIZING_GROW(0);
        CLAY({.layout = {.sizing = {data.vertical ? CLAY_SIZING_FIXED(1) : length,
            data.vertical ? length : CLAY_SIZING_FIXED(1)}}, .backgroundColor = context_.theme.border}) {}
    }
private: UI::Context& context_;
};

enum class BadgeTone { Neutral, Accent, Danger };
class Badge : public Component {
public:
    struct Data { std::string label; BadgeTone tone = BadgeTone::Neutral; bool visible = true; } data;
    Badge(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}
    void renderComponent() override {
        if (!data.visible) return;
        auto& t = context_.theme;
        const auto color = data.tone == BadgeTone::Danger ? t.danger : data.tone == BadgeTone::Accent ? t.accent : t.text;
        CLAY({.layout = {.padding = {8,8,4,4}}, .backgroundColor = t.subtle,
            .cornerRadius = CLAY_CORNER_RADIUS(4)}) { context_.text(data.label,color,13); }
    }
private: UI::Context& context_;
};

class ImageView : public Component {
public:
    // imageData must come from Renderer::uploadImage(...).clayData().
    // The renderer owns the texture and must outlive this view.
    struct Data {
        std::string id;
        void* imageData = nullptr;
        float width = 128, height = 128, radius = 0;
        bool visible = true;
    } data;
    ImageView(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}
    void renderComponent() override {
        if (!data.visible) return;
        CLAY({.id = UI::elementId(data.id), .layout = {.sizing = {CLAY_SIZING_FIXED(std::max(1.0f,data.width)),
            CLAY_SIZING_FIXED(std::max(1.0f,data.height))}},
            .backgroundColor = data.imageData ? Clay_Color{255,255,255,255} : context_.theme.subtle,
            .cornerRadius = CLAY_CORNER_RADIUS(data.radius), .image = {.imageData = data.imageData}}) {}
    }
private: UI::Context& context_;
};
}
