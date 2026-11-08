module;
#include <cstddef>
#include <meta>
#include <type_traits>
#include <utility> // for std::index_sequence

export module scrivlr;

export namespace Scrivlr {

template <class T>
    requires(std::is_class_v<T> && std::is_aggregate_v<T>)
struct StructuralHash {
    std::size_t operator()(const T& obj) const {
        static constexpr auto MEMBERS =
            std::define_static_array(std::meta::nonstatic_data_members_of(
                ^^T, std::meta::access_context::current()));

        std::size_t seed = 0;

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            auto hash_member = [&]<std::meta::info M>() {
                const auto& member_val = obj.[:M:];
                const std::size_t MEMBER_HASH =
                    std::hash<std::remove_cvref_t<decltype(member_val)>>{}(
                        member_val);
                seed ^= MEMBER_HASH + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            };

            (hash_member.template operator()<MEMBERS[Is]>(), ...);
        }(std::make_index_sequence<MEMBERS.size()>{});

        return seed;
    }
};
} // namespace Scrivlr
