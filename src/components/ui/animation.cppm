module;
#include <algorithm>
#include <cmath>
export module scrivlr_animation;

export namespace Scrivlr::UI {
// A retargetable transition. Retargeting preserves the displayed value.
class Animation {
public:
    explicit Animation(float initial = 0)
        : value_(initial), start_(initial), target_(initial) {}
    void target(float value, float duration = 0.14f) {
        if (value == target_)
            return;
        start_ = value_;
        target_ = value;
        elapsed_ = 0;
        duration_ = std::max(duration, 0.0f);
    }
    float update(float dt, bool reduced_motion = false) {
        if (reduced_motion || duration_ == 0)
            return value_ = target_;
        elapsed_ = std::min(duration_, elapsed_ + std::max(0.0f, dt));
        const float t = elapsed_ / duration_;
        const float eased = t == 1 ? 1 : 1 - std::pow(2.0f, -10.0f * t);
        value_ = start_ + (target_ - start_) * eased;
        return value_;
    }
    float value() const { return value_; }
    bool active() const { return value_ != target_; }

private:
    float value_, start_, target_, elapsed_ = 0, duration_ = 0;
};
} // namespace Scrivlr::UI
