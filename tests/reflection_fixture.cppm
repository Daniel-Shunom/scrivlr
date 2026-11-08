module;

#include <meta>

export module reflection_fixture;

export namespace ReflectionProbe {
struct Generated;
consteval {
    std::meta::define_aggregate(^^Generated,
        {std::meta::data_member_spec(^^int,
            {
                .name = "value",
            })});
}
} // namespace ReflectionProbe
