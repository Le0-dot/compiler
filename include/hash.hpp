#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>
#include <vector>


namespace {

template<typename T>
concept integral = 
    std::integral<T> ||
    (std::is_enum_v<T> && std::integral<std::underlying_type_t<T>>);

template<typename T>
requires integral<T>
struct integral_type {
    using type = std::underlying_type_t<T>;
};

template<typename T>
requires std::integral<T>
struct integral_type<T> {
    using type = T;
};

template<typename T>
using integral_type_t = typename integral_type<T>::type;

} // namespace


template<typename T, typename U>
struct pair_hash;

template<typename T>
requires integral<T>
struct vector_hash;

template<typename T>
struct hash_for {
    using type = std::hash<T>;
};

template<typename T>
struct hash_for<std::vector<T>> {
    using type = vector_hash<T>;
};

template<typename T, typename U>
struct hash_for<std::pair<T, U>> {
    using type = pair_hash<T, U>;
};

template<typename T>
using hash_for_t = typename hash_for<T>::type;


template<typename T>
requires integral<T>
struct vector_hash {
    auto operator()(const std::vector<T>& vec) const noexcept -> std::size_t {
	using int_t = integral_type_t<T>;

	std::size_t seed = vec.size();
	for(const auto& elem : vec) {
	    seed ^= static_cast<int_t>(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
    }
};

template<typename T, typename U>
struct pair_hash {
    auto operator()(const std::pair<T, U>& pair) const noexcept -> std::size_t {
	uintmax_t hash = hash_for_t<T>{}(pair.first);
        hash <<= sizeof(uintmax_t) * 4;
        hash ^= hash_for_t<U>{}(pair.second);
        return std::hash<uintmax_t>{}(hash);
    }
};
