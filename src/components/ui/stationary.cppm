module;
#include <clay.h>
#include <string>

export module scrivlr_widget_stationary;

export import scrivlr_widget_context;
import components;

export namespace Scrivlr::Components {
enum class StationaryKind {
    Pencil,
    Brush,
    Pen,
    Eraser,
};

class Stationary : public Component {
public:
    struct Data {
        std::string id;
        StationaryKind kind = StationaryKind::Pen;
        float size = 10.0f;
        bool is_active = false;
    } data;
    void renderComponent() override {
        CLAY({
            .id = UI::elementId(data.id),
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .height = CLAY_SIZING_FIXED(32), .width = CLAY_SIZING_FIXED(32)
                }
            },
        }) {
            switch (data.kind) {
            case StationaryKind::Brush:
                // Load and use rasterized Brush image
                break;

            case StationaryKind::Pencil:
                // Load and use rasterized Pencil image
                break;

            case StationaryKind::Pen:
                // Load and use rasterized Pen image
                break;

            case StationaryKind::Eraser:
                // Load and use rasterized Eraser image
                break;
            }
        }
    }
};
} // namespace Scrivlr::Components
