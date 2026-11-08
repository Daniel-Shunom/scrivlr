module;

#include <unordered_map>
#include <memory>
#include <utility>
#include <meta>
export module component_registry;

import components;
import scrivlr;

export namespace Scrivlr::Components {

class ComponentRegistry {
public:
    bool insert(ComponentID id, std::unique_ptr<Component> component) {
        if (!component) return false;
        return registry.try_emplace(id, std::move(component)).second;
    }

    bool contains(ComponentID id) const { return registry.contains(id); }

    Component* find(ComponentID id) {
        auto entry = registry.find(id);
        return entry == registry.end() ? nullptr : entry->second.get();
    }

    const Component* find(ComponentID id) const {
        auto entry = registry.find(id);
        return entry == registry.end() ? nullptr : entry->second.get();
    }

    bool erase(ComponentID id) { return registry.erase(id) != 0; }
    std::size_t size() const { return registry.size(); }

private:
    std::unordered_map<ComponentID, std::unique_ptr<Component>, Scrivlr::StructuralHash<ComponentID>> registry;
};

}
