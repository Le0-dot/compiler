#pragma once

#include <limits>
#include <cstdint>
#include <type_traits>


namespace type {

enum class type_id : std::uint64_t {
    undetermined,
    unset,
    good_file, 
    good_stmt,

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
    i8,
    i16,
    i32,
    i64,
    fp32,
    fp64,
    primitive_bound,
};

constexpr inline auto operator++(type_id& tid) -> type_id& {
    using type_id_underlying = std::underlying_type_t<type_id>;
    return tid = static_cast<type_id>(static_cast<type_id_underlying>(tid) + 1);
}

constexpr inline auto valid(const type_id& tid) -> bool {
    return tid != type_id::undetermined;
}

constexpr inline auto is_literal(type_id tid) -> bool {
    return type_id::u_literal <= tid && tid < type_id::literal_bound;
}

constexpr inline auto default_type(type_id tid) -> type_id {
    switch(tid) {
	case type_id::u_literal:
	    return type_id::u32;
	case type_id::i_literal:
	    return type_id::i32;
	case type_id::fp_literal:
	    return type_id::fp64;
	default:
	    return tid;
    };
}

} // namespace type
