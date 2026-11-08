module;
#include <clay.h>
#include <string>
export module component_workspace;
import scrivlr_components;

export namespace Scrivlr::Components {
class Workspace : public Component {
public:
    struct Data { int width = 1280, height = 800, additions = 0; } data;
    UI::Context context;
    Button add{context,{.id="workspace/add",.label="Add shape",.onClick=[this] {
        ++data.additions;
        const float offset=static_cast<float>(data.additions);
        canvas.data.shapes.push_back({{60+20*offset,60+15*offset},100,72,{37,87,170,255},8});
    }}};
    Button reset{context,{.id="workspace/reset",.label="Reset view",.variant=ButtonVariant::Secondary,
        .onClick=[this] { canvas.resetView(); }}};
    Button disabled{context,{.id="workspace/disabled",.label="Unavailable",.variant=ButtonVariant::Secondary,.enabled=false}};
    Checkbox grid{context,{.id="workspace/grid",.label="Show grid",.checked=true,
        .onChange=[this](bool value) { canvas.data.grid=value; }}};
    Switch motion{context,{.id="workspace/motion",.label="Reduce motion",
        .onChange=[this](bool value) { context.reduced_motion=value; }}};
    Slider opacity{context,{.id="workspace/opacity",.label="Shape opacity",.value=1,.minimum=0.1f,.maximum=1,
        .onChange=[this](float value) { for (auto& shape : canvas.data.shapes) shape.color.a=255*value; progress.data.value=value; }}};
    Progress progress{context,{.id="workspace/progress",.value=1}};
    Tabs tabs{context,{.id="workspace/tabs",.labels={"Canvas","Components"}}};
    Canvas canvas{context,{.id="workspace/canvas",.shapes={
        {{48,48},150,100,{37,87,170,255},8}, {{236,118},110,110,{35,39,46,255},55},
        {{390,60},130,160,{144,169,205,255},8}}}};
    Icon plus{context,{.id="workspace/icon/plus",.kind=IconKind::Plus}};
    Icon check{context,{.id="workspace/icon/check",.kind=IconKind::Check}};
    Icon menu{context,{.id="workspace/icon/menu",.kind=IconKind::Menu}};
    Icon close{context,{.id="workspace/icon/close",.kind=IconKind::Close}};
    Icon arrow{context,{.id="workspace/icon/arrow",.kind=IconKind::ArrowRight}};
    Label title{context,{.text="Scrivlr",.size=24}};
    Label subtitle{context,{.text="Component library",.muted=true}};
    Label controls_title{context,{.text="Controls",.size=20}};
    Label canvas_title{context,{.text="Canvas",.size=20}};
    Label help{context,{.text="Drag to pan. Scroll to zoom. Tab to focus controls.",.size=14,.muted=true}};
    Label component_help{context,{.text="Components own Data and compose through renderComponent().",.muted=true}};
    Label canvas_hint{context,{.text="Add a shape, adjust its opacity, or hide the grid.",.size=14,.muted=true}};
    Badge badge{context,{.label="Interactive",.tone=BadgeTone::Accent}};
    Separator separator{context};
    void renderComponent() override {
        auto& t=context.theme;
        const bool wide=data.width>=850;
        canvas.data.height=wide ? static_cast<float>(data.height)-260 : 260;
        CLAY({.id=CLAY_ID("Workspace"),.layout={.sizing={CLAY_SIZING_GROW(0),CLAY_SIZING_GROW(0)},
            .padding=CLAY_PADDING_ALL(24),.childGap=20,.layoutDirection=CLAY_TOP_TO_BOTTOM},
            .backgroundColor=t.background,.clip={.vertical=true,.childOffset=Clay_GetScrollOffset()}}) {
            UI::row([&] { title.renderComponent(); subtitle.renderComponent(); badge.renderComponent(); });
            tabs.renderComponent();
            CLAY({.layout={.sizing={CLAY_SIZING_GROW(0),CLAY_SIZING_FIT(0)},.childGap=24,
                .layoutDirection=wide ? CLAY_LEFT_TO_RIGHT : CLAY_TOP_TO_BOTTOM}}) {
                CLAY({.layout={.sizing={wide ? CLAY_SIZING_FIXED(294) : CLAY_SIZING_GROW(0),CLAY_SIZING_FIT(0)},
                    .padding=CLAY_PADDING_ALL(20),.childGap=12,.layoutDirection=CLAY_TOP_TO_BOTTOM},
                    .backgroundColor=t.surface,.cornerRadius=CLAY_CORNER_RADIUS(8)}) {
                    controls_title.renderComponent();
                    UI::row([&] { add.renderComponent(); reset.renderComponent(); },8);
                    disabled.renderComponent(); separator.renderComponent(); grid.renderComponent(); motion.renderComponent();
                    opacity.renderComponent(); progress.renderComponent();
                    UI::row([&] { plus.renderComponent(); check.renderComponent(); menu.renderComponent(); close.renderComponent(); arrow.renderComponent(); });
                }
                UI::column([&] {
                    if (tabs.data.selected==0) {
                        canvas_title.renderComponent(); help.renderComponent(); canvas.renderComponent(); canvas_hint.renderComponent();
                    } else { component_help.renderComponent(); composition.renderComponent(); }
                });
            }
        }
    }
    Workspace() {
        composition.emplaceChild<Label>(context,Label::Data{.text="Composed panel",.size=20});
        auto& message=composition.emplaceChild<Label>(context,Label::Data{.text="These controls are owned child components.",.muted=true});
        composition.emplaceChild<Checkbox>(context,Checkbox::Data{.id="workspace/nested/check",.label="Local component state"});
        auto& action=composition.emplaceChild<Button>(context,Button::Data{.id="workspace/nested/button",.label="Button with an icon",.variant=ButtonVariant::Secondary,
            .onClick=[message=&message] { message->data.text="The child button updated its sibling's Data."; }});
        action.emplaceChild<Icon>(context,Icon::Data{.id="workspace/nested/icon",.kind=IconKind::ArrowRight});
    }
private:
    Panel composition{context,{.id="workspace/composition"}};
};
void buildWorkspace(void* data) {
    if (data) static_cast<Workspace*>(data)->renderComponent();
    else {
        static Workspace preview;
        preview.context.beginFrame({}); preview.renderComponent(); preview.context.endFrame();
    }
}
}
