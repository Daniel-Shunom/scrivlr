module;

#include <clay.h>

export module component_workspace;

export namespace Scrivlr::Components {
void buildWorkspace(void*) {
    CLAY({.id = CLAY_ID("Workspace"),
        .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .padding = CLAY_PADDING_ALL(24), .childGap = 16, .layoutDirection = CLAY_TOP_TO_BOTTOM},
        .backgroundColor = {245, 245, 245, 255}}) {
        CLAY({.id = CLAY_ID("Toolbar"),
            .layout = {.sizing = {.width = CLAY_SIZING_GROW(0)}, .padding = CLAY_PADDING_ALL(16)},
            .backgroundColor = {35, 39, 46, 255}, .cornerRadius = CLAY_CORNER_RADIUS(8)}) {
            CLAY_TEXT(CLAY_STRING("Scrivlr"), CLAY_TEXT_CONFIG({.textColor = {255, 255, 255, 255}, .fontSize = 24}));
        }
        CLAY({.id = CLAY_ID("Canvas"),
            .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .padding = CLAY_PADDING_ALL(24)},
            .backgroundColor = {255, 255, 255, 255}, .cornerRadius = CLAY_CORNER_RADIUS(8),
            .border = {.color = {210, 210, 210, 255}, .width = CLAY_BORDER_ALL(1)}}) {
            CLAY_TEXT(CLAY_STRING("Your components go here."), CLAY_TEXT_CONFIG({.textColor = {80, 80, 80, 255}, .fontSize = 18}));
        }
    }
}
}
