module;
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>
export module components;

export namespace Scrivlr::Components {
class Component {
public:
    virtual ~Component() = default;
    virtual void renderComponent() = 0;
    Component& addChild(std::unique_ptr<Component> child) {
        if (child) children_.push_back(std::move(child));
        return *this;
    }
    template<class T, class... Args> T& emplaceChild(Args&&... args) {
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        auto& result = *child;
        children_.push_back(std::move(child));
        return result;
    }
    std::size_t childCount() const { return children_.size(); }
protected:
    void renderChildren() { for (auto& child : children_) child->renderComponent(); }
private:
    std::vector<std::unique_ptr<Component>> children_;

};

class ComponentID {
public:
    std::uintptr_t id{};
    bool operator==(const ComponentID&) const = default;
};

}
