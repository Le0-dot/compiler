#pragma once

#include <limits>
#include <cstdint>
#include <type_traits>


namespace type {

enum class type_id : std::uint64_t {
    undetermined,

    u_literal,
    i_literal,
    fp_literal,
    literal_bound,

    void_,
    bool_,
    char_,
    u8,
    u16,
    u32,
    u64,
    u128,
    i8,
    i16,
    i32,
    i64,
    i128,
    fp32,
    fp64,
    fp128,
    primitive_bound,
};

constexpr inline type_id& operator++(type_id& id) {
    using type_id_underlying = std::underlying_type_t<type_id>;
    return id = static_cast<type_id>(static_cast<type_id_underlying>(id) + 1);
}

}
