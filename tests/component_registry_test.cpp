#include <string>
#include <memory>
#include <type_traits>
#include <meta>

import scrivlr;
import components;
import component_registry;

struct Record {
    int number;
    std::string label;
};

struct TestComponent : Scrivlr::Components::Component {
    int& draws;
    int& destructions;
    TestComponent(int& draws, int& destructions) : draws(draws), destructions(destructions) {}
    ~TestComponent() override { ++destructions; }
    void renderComponent() override { ++draws; }
};

int main() {
    using namespace Scrivlr::Components;
    static_assert(std::is_copy_constructible_v<Scrivlr::StructuralHash<ComponentID>>);
    ComponentRegistry registry;
    int draws = 0;
    int destructions = 0;
    if (registry.size() != 0 || registry.contains({1}) || registry.find({1})) return 1;
    if (!registry.insert({1}, std::make_unique<TestComponent>(draws, destructions)) ||
        !registry.insert({2}, std::make_unique<TestComponent>(draws, destructions))) return 2;
    if (registry.insert({1}, std::make_unique<TestComponent>(draws, destructions)) || registry.size() != 2) return 3;
    if (!registry.contains({1}) || !registry.find({2})) return 4;
    registry.find({2})->renderComponent();
    if (draws != 1 || destructions != 1) return 9;
    const auto& view = registry;
    if (!view.find({1}) || view.find({3})) return 5;
    if (!registry.erase({1}) || registry.erase({1}) || registry.size() != 1) return 6;
    if (destructions != 2) return 10;
    if (registry.insert({3}, nullptr) || registry.contains({3})) return 11;
    if (!registry.erase({2}) || destructions != 3) return 12;
    if (ComponentID{}.id != 0 || !(ComponentID{5} == ComponentID{5}) || ComponentID{5} == ComponentID{6}) return 7;
    Scrivlr::StructuralHash<Record> hash;
    if (hash({42, "canvas"}) != hash({42, "canvas"})) return 8;
    return 0;
}
