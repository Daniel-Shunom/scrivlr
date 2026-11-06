module;

#include <cstddef>

export module scrivlr;

export namespace Scrivlr {

struct BoundingBox {
    int x_start{};
    int y_start{};
    int x_len{};
    int y_width{};
};

struct Drawing {
    std::size_t size{};
    BoundingBox box{};
};

} // namespace scrivlr
