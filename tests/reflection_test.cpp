#include <meta>
#include <type_traits>

import reflection_fixture;

enum class Component { Toolbar, Canvas };
using ReflectedComponent = [:^^Component:];
static_assert(std::is_same_v<Component, ReflectedComponent>);
static_assert(std::meta::identifier_of(^^Component::Canvas) == "Canvas");

int main() {
    ReflectionProbe::Generated generated{42};
    return generated.value == 42 ? 0 : 1;
}
