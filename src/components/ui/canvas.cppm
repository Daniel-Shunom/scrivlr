module;
#include <algorithm>
#include <clay.h>
#include <cmath>
#include <string>
#include <utility>
#include <vector>
export module scrivlr_canvas;
export import scrivlr_widget_context;
import components;

export namespace Scrivlr::UI {
struct Point {
        float x = 0, y = 0;
};

inline void rectangle(float x, float y, float width, float height, Clay_Color color,
                      float radius = 0) {
    if (width <= 0 || height <= 0)
        return;
    CLAY({.layout = {.sizing = {CLAY_SIZING_FIXED(width), CLAY_SIZING_FIXED(height)}},
          .backgroundColor = color,
          .cornerRadius = CLAY_CORNER_RADIUS(radius),
          .floating = {.offset = {x, y},
                       .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                       .attachTo = CLAY_ATTACH_TO_PARENT,
                       .clipTo = CLAY_CLIP_TO_ATTACHED_PARENT}}) {}
}
} // namespace Scrivlr::UI

export namespace Scrivlr::Components {

enum class IconKind { Plus, Minus, Close, Menu, Canvas, Check, ArrowRight };

class Icon : public Component {
    public:
        struct Data {
                std::string id;
                IconKind kind = IconKind::Plus;
                float size = 20;
                bool visible = true;
        } data;
        Icon(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}
        void renderComponent() override {
            if (!data.visible) {
                return;
            }

            const float size = std::max(8.0f, data.size), u = size / 20;

            const auto color = context_.theme.text;

            CLAY({.id = UI::elementId(data.id),
                  .layout = {.sizing = {CLAY_SIZING_FIXED(size), CLAY_SIZING_FIXED(size)}}}) {
                auto stroke = [&](float x, float y, float w, float h) {
                    UI::rectangle(x * u, y * u, w * u, h * u, color, u);
                };
                switch (data.kind) {
                case IconKind::Plus:
                    stroke(9, 3, 2, 14);
                    [[fallthrough]];

                case IconKind::Minus:
                    stroke(3, 9, 14, 2);
                    break;

                case IconKind::Menu:
                    for (int i = 0; i < 3; ++i) {
                        stroke(3, 4 + static_cast<float>(i) * 5, 14, 2);
                    }
                    break;

                case IconKind::Canvas:
                    stroke(3, 3, 14, 2);
                    stroke(3, 15, 14, 2);
                    stroke(3, 3, 2, 14);
                    stroke(15, 3, 2, 14);
                    break;

                case IconKind::Close:
                    for (int i = 0; i < 11; ++i) {
                        const float p = static_cast<float>(i);
                        stroke(4 + p, 4 + p, 2, 2);
                        stroke(14 - p, 4 + p, 2, 2);
                    }
                    break;

                case IconKind::Check:
                    for (int i = 0; i < 5; ++i) {
                        const float p = static_cast<float>(i);
                        stroke(3 + p, 9 + p, 2, 2);
                    }
                    for (int i = 0; i < 9; ++i) {
                        const float p = static_cast<float>(i);
                        stroke(7 + p, 13 - p, 2, 2);
                    }
                    break;

                case IconKind::ArrowRight:
                    stroke(3, 9, 13, 2);
                    for (int i = 0; i < 6; ++i) {
                        const float p = static_cast<float>(i);
                        stroke(10 + p, 4 + p, 2, 2);
                        stroke(10 + p, 14 - p, 2, 2);
                    }
                    break;
                }
            }
        }

    private:
        UI::Context& context_;
};

class Canvas : public Component {
    public:
        struct Shape {
                UI::Point position;
                float width = 100, height = 80;
                Clay_Color color{37, 87, 170, 255};
                float radius = 6;
        };

        struct Data {
                std::string id;
                float height = 260;
                UI::Point pan{};
                float zoom = 1;
                bool grid = true, enabled = true, visible = true;
                std::vector<Shape> shapes;
        } data;

        Canvas(UI::Context& context, Data value) : data(std::move(value)), context_(context) {}

        UI::Point toWorld(UI::Point local) const {
            return {(local.x - data.pan.x) / data.zoom, (local.y - data.pan.y) / data.zoom};
        }

        UI::Point toScreen(UI::Point world) const {
            return {world.x * data.zoom + data.pan.x, world.y * data.zoom + data.pan.y};
        }

        void resetView() {
            data.pan = {};
            data.zoom = 1;
        }

        bool containsPoint(float x, float y) const {
            return data.visible && data.enabled && bounds_.width > 0 && x >= bounds_.x &&
                   y >= bounds_.y && x < bounds_.x + bounds_.width &&
                   y < bounds_.y + bounds_.height;
        }

        void renderComponent() override {
            if (!data.visible) {
                return;
            }

            data.zoom = std::clamp(data.zoom, 0.25f, 4.0f);
            const auto id = UI::elementId(data.id);
            const auto s = context_.interact(id, data.enabled);
            const auto box = Clay_GetElementData(id);

            if (box.found) {
                bounds_ = box.boundingBox;
            }

            if (s.held && dragging_) {
                data.pan.x += context_.input.x - last_.x;
                data.pan.y += context_.input.y - last_.y;
            }

            dragging_ = s.held;
            last_ = {context_.input.x, context_.input.y};

            if (s.hovered && data.enabled && box.found && context_.input.wheel != 0) {
                const UI::Point local{context_.input.x - box.boundingBox.x,
                                      context_.input.y - box.boundingBox.y};
                const auto anchor = toWorld(local);
                data.zoom =
                    std::clamp(data.zoom * std::pow(1.1f, context_.input.wheel), 0.25f, 4.0f);
                data.pan = {local.x - anchor.x * data.zoom, local.y - anchor.y * data.zoom};
            }

            if (s.focused) {
                if (context_.input.left) {
                    data.pan.x -= 16;
                }

                if (context_.input.right) {
                    data.pan.x += 16;
                }
            }

            auto& t = context_.theme;

            CLAY({.id = id,
                  .layout = {.sizing = {CLAY_SIZING_GROW(0),
                                        CLAY_SIZING_FIXED(std::max(40.0f, data.height))}},
                  .backgroundColor = t.background,
                  .clip = {.horizontal = true, .vertical = true},
                  .border = {.color = s.focused ? t.accent : t.border,
                             .width = UI::outline(static_cast<uint16_t>(s.focused ? 2 : 1))}}) {
                // Previous layout bounds are used for culling; first frame gets a conservative
                // width.
                const float width = box.found ? box.boundingBox.width : 1200;
                if (data.grid) {
                    const float spacing = 32 * data.zoom;
                    for (float x = std::fmod(data.pan.x, spacing); x < width; x += spacing)
                        UI::rectangle(x, 0, 1, data.height, t.border);
                    for (float y = std::fmod(data.pan.y, spacing); y < data.height; y += spacing)
                        UI::rectangle(0, y, width, 1, t.border);
                }
                for (const auto& shape : data.shapes) {
                    const auto p = toScreen(shape.position);
                    if (p.x > width || p.y > data.height || p.x + shape.width * data.zoom < 0 ||
                        p.y + shape.height * data.zoom < 0)
                        continue;
                    UI::rectangle(p.x, p.y, shape.width * data.zoom, shape.height * data.zoom,
                                  shape.color, shape.radius * data.zoom);
                }
            }
        }

    private:
        UI::Context& context_;
        bool dragging_ = false;
        UI::Point last_;
        Clay_BoundingBox bounds_{};
};
} // namespace Scrivlr::Components
