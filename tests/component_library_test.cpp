#include <clay.h>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>
import scrivlr_components;
import component_workspace;

using namespace Scrivlr;
using namespace Scrivlr::Components;

struct Probe : Component {
    int& count;
    explicit Probe(int& count) : count(count) {}
    void renderComponent() override { ++count; }
};

int main() {
    bool error=false;
    std::vector<unsigned char> memory(Clay_MinMemorySize());
    Clay_Initialize(Clay_CreateArenaWithCapacityAndMemory(memory.size(),memory.data()),{800,600},
        {.errorHandlerFunction=[](Clay_ErrorData e) { *static_cast<bool*>(e.userData)=true;
            std::fprintf(stderr,"Clay: %.*s\n",e.errorText.length,e.errorText.chars); },.userData=&error});
    Clay_SetMeasureTextFunction(+[](Clay_StringSlice text,Clay_TextElementConfig*,void*) -> Clay_Dimensions {
        return {static_cast<float>(text.length)*8,16};
    },nullptr);
    UI::Context context;
    int clicks=0;
    Button button(context,{.id="test/button",.label="Save",.onClick=[&] { ++clicks; }});
    auto frame=[&](Component& component,UI::Input input) {
        context.beginFrame(input);
        Clay_SetPointerState({input.x,input.y},input.down);
        Clay_BeginLayout(); component.renderComponent(); auto commands=Clay_EndLayout();
        context.endFrame(); return commands;
    };
    auto require=[&](bool condition,const char* message) { if (!condition) std::fprintf(stderr,"%s\n",message); return condition; };
    frame(button,{});
    frame(button,{.x=10,.y=10,.down=true});
    frame(button,{.x=10,.y=10});
    if (!require(clicks==1,"Button should activate once on release")) return 1;
    frame(button,{.x=10,.y=10});
    if (!require(clicks==1,"Idle frames must not repeat activation")) return 2;
    frame(button,{.x=10,.y=10,.down=true});
    frame(button,{.x=700,.y=500});
    if (!require(clicks==1,"Release outside must cancel activation")) return 3;
    button.data.enabled=false;
    frame(button,{.x=10,.y=10,.down=true}); frame(button,{.x=10,.y=10});
    if (!require(clicks==1,"Disabled button must ignore input")) return 4;
    button.data.enabled=true;
    frame(button,{}); frame(button,{.tab=true}); frame(button,{.activate=true});
    if (!require(clicks==2,"Keyboard activation must work")) return 5;
    Checkbox checkbox(context,{.id="test/check",.label="Grid"});
    frame(checkbox,{}); frame(checkbox,{.x=10,.y=10,.down=true}); frame(checkbox,{.x=10,.y=10});
    if (!require(checkbox.data.checked,"Checkbox must retain its own state")) return 6;
    int renders=0;
    Panel panel(context,{.id="test/panel"});
    panel.emplaceChild<Probe>(renders); panel.addChild(std::make_unique<Probe>(renders));
    frame(panel,{});
    if (!require(renders==2 && panel.childCount()==2,"Panel must render owned children")) return 7;
    UI::Animation animation;
    animation.target(1); const float halfway=animation.update(0.07f);
    animation.target(0);
    if (!require(halfway>0 && halfway<1 && animation.update(0)==halfway,"Retarget must preserve current value")) return 8;
    if (!require(animation.update(0,true)==0 && !animation.active(),"Reduced motion must settle immediately")) return 9;
    Canvas canvas(context,{.id="test/canvas",.pan={10,20},.zoom=2});
    auto screen=canvas.toScreen({30,40}); auto world=canvas.toWorld(screen);
    if (!require(world.x==30 && world.y==40,"Canvas transforms must round-trip")) return 10;
    canvas.data.zoom=100; frame(canvas,{});
    if (!require(canvas.data.zoom==4,"Canvas zoom must be bounded")) return 11;
    Slider slider(context,{.id="test/slider",.label="Opacity",.value=0.5f});
    frame(slider,{}); frame(slider,{.tab=true}); frame(slider,{.right=true});
    if (!require(slider.data.value>0.5f,"Focused slider must support arrow keys")) return 12;
    Tabs tabs(context,{.id="test/tabs",.labels={"One","Two"}});
    frame(tabs,{}); frame(tabs,{.tab=true}); frame(tabs,{.tab=true}); frame(tabs,{.activate=true});
    if (!require(tabs.data.selected==1,"Tabs must support keyboard selection")) return 13;
    // Exercise the actual application's composed controls with real Clay hit testing.
    Workspace workspace;
    Clay_SetLayoutDimensions({1280,800});
    auto appFrame=[&](UI::Input input) {
        workspace.context.beginFrame(input);
        Clay_SetPointerState({input.x,input.y},input.down);
        Clay_BeginLayout(); workspace.renderComponent(); Clay_EndLayout();
        workspace.context.endFrame();
    };
    auto click=[&](const std::string& id) {
        auto box=Clay_GetElementData(UI::elementId(id)).boundingBox;
        appFrame({.x=box.x+box.width/2,.y=box.y+box.height/2,.down=true});
        appFrame({.x=box.x+box.width/2,.y=box.y+box.height/2});
    };
    appFrame({});
    click("workspace/add");
    if (!require(workspace.canvas.data.shapes.size()==4,"Playground Add shape must update canvas data")) return 15;
    click("workspace/grid");
    if (!require(!workspace.canvas.data.grid,"Playground checkbox must update the canvas")) return 16;
    click("workspace/motion");
    if (!require(workspace.context.reduced_motion,"Playground switch must update reduced motion")) return 17;
    const auto sliderBox=Clay_GetElementData(UI::elementId("workspace/opacity")).boundingBox;
    appFrame({.x=sliderBox.x+sliderBox.width/2,.y=sliderBox.y+14,.down=true});
    appFrame({.x=-100,.y=sliderBox.y+14,.down=true});
    appFrame({.x=-100,.y=sliderBox.y+14});
    if (!require(workspace.opacity.data.value==workspace.opacity.data.minimum &&
        workspace.progress.data.value==workspace.opacity.data.value,"Slider capture and bindings must survive dragging outside")) return 18;
    const auto canvasBox=Clay_GetElementData(UI::elementId("workspace/canvas")).boundingBox;
    const UI::Point anchor{80,80};
    const auto before=workspace.canvas.toWorld(anchor);
    appFrame({.x=canvasBox.x+80,.y=canvasBox.y+80,.wheel=2});
    const auto after=workspace.canvas.toWorld(anchor);
    if (!require(workspace.canvas.data.zoom>1 && std::abs(before.x-after.x)<0.001f &&
        std::abs(before.y-after.y)<0.001f,"Zoom must preserve the world position under the pointer")) return 19;
    click("workspace/reset");
    if (!require(workspace.canvas.data.zoom==1 && workspace.canvas.data.pan.x==0,"Reset view must restore canvas transform")) return 20;
    click("workspace/tabs/1");
    if (!require(workspace.tabs.data.selected==1 && Clay_GetElementData(UI::elementId("workspace/nested/button")).found,
        "Components tab must render the composed subtree")) return 21;
    if (!require(!error,"Layouts must not produce Clay errors")) return 14;
    Clay_SetCurrentContext(nullptr);
    std::puts("Component input, state, composition, animation and canvas tests passed");
}
