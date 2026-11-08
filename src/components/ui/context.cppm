module;
#include <algorithm>
#include <clay.h>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>
export module scrivlr_widget_context;

export namespace Scrivlr::UI {
struct Theme {
    Clay_Color background{245, 245, 245, 255}, surface{255, 255, 255, 255};
    Clay_Color text{35, 39, 46, 255}, muted{86, 94, 105, 255},
        border{210, 215, 222, 255};
    Clay_Color accent{37, 87, 170, 255}, accent_hover{29, 68, 138, 255};
    Clay_Color subtle{232, 237, 245, 255}, disabled{225, 228, 232, 255},
        danger{173, 41, 49, 255};
    uint16_t font_size = 16, gap = 12, padding = 16;
    float radius = 6;
};
struct Input {
    float x = -1, y = -1, dt = 0, wheel = 0;
    bool down = false, tab = false, shift = false, activate = false;
    bool left = false, right = false, escape = false;
};
struct Interaction {
    bool hovered = false, held = false, clicked = false, focused = false;
};

class Context {
public:
    Theme theme;
    Input input;
    bool reduced_motion = false;
    void beginFrame(Input next) {
        frame_text_.clear();
        pressed_ = next.down && !input.down;
        released_ = !next.down && input.down;
        if (next.tab && !order_.empty()) {
            auto it = std::find(order_.begin(), order_.end(), focused_);
            auto index = it == order_.end() ? (next.shift ? 0 : -1)
                                            : static_cast<int>(it - order_.begin());
            index =
                (index + (next.shift ? -1 : 1) + static_cast<int>(order_.size())) %
                static_cast<int>(order_.size());
            focused_ = order_[static_cast<std::size_t>(index)];
        }
        if (next.escape) {
            focused_ = 0;
            captured_ = 0;
        }
        order_.clear();
        input = next;
    }
    void endFrame() {
        if (released_ || !input.down)
            captured_ = 0;
        if (std::find(order_.begin(), order_.end(), focused_) == order_.end())
            focused_ = 0;
    }
    Interaction interact(Clay_ElementId id, bool enabled) {
        const bool hover = Clay_PointerOver(id);
        if (!enabled) {
            if (captured_ == id.id)
                captured_ = 0;
            return {};
        }
        order_.push_back(id.id);
        if (pressed_ && hover && captured_ == 0) {
            captured_ = id.id;
            focused_ = id.id;
        }
        return {
            hover,
            captured_ == id.id && input.down,
            (released_ && captured_ == id.id && hover) ||
                (focused_ == id.id && input.activate),
            focused_ == id.id
        };
    }
    // Clay consumes strings after layout. Snapshot text so callbacks may change Data
    // safely.
    void text(const std::string& value, Clay_Color color, uint16_t size = 16) {
        frame_text_.push_back(value);
        const auto& stored = frame_text_.back();
        CLAY_TEXT(
            (Clay_String{false, static_cast<int32_t>(stored.size()), stored.data()}),
            CLAY_TEXT_CONFIG({
                .textColor = color,
                .fontSize = size,
            }));
    }

private:
    std::deque<std::string> frame_text_;
    bool pressed_ = false, released_ = false;
    uint32_t captured_ = 0, focused_ = 0;
    std::vector<uint32_t> order_;
};

inline Clay_String textView(const std::string& text) {
    return {false, static_cast<int32_t>(text.size()), text.data()};
}
inline Clay_ElementId elementId(const std::string& id) {
    return Clay_GetElementId(textView(id));
}
inline void text(const std::string& value, Clay_Color color, uint16_t size = 16) {
    CLAY_TEXT(textView(value),
        CLAY_TEXT_CONFIG({
            .textColor = color,
            .fontSize = size,
        }));
}
inline Clay_Color mix(Clay_Color a, Clay_Color b, float t) {
    return {
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    };
}
inline Clay_BorderWidth outline(uint16_t width) {
    return {width, width, width, width, 0};
}
// Composition stays compatible with ordinary CLAY blocks.
template <class Children> void column(Children children, uint16_t gap = 12) {
    CLAY({
        .layout = {
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)},
            .childGap = gap,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    }) {
        children();
    }
}
template <class Children> void row(Children children, uint16_t gap = 12) {
    CLAY({
        .layout = {
            .childGap = gap,
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
    }) {
        children();
    }
}
} // namespace Scrivlr::UI
