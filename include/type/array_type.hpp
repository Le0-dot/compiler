#pragma once

#include <memory>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type_id.hpp"


namespace type {

class array_type {
    llvm::ArrayType* _type{};
    type_id _element{};
    std::size_t _length{};

public:
    array_type(llvm::ArrayType* type, type_id element, std::size_t length) noexcept 
	: _type{type}
	, _element{element}
	, _length{length}
    {}

    array_type()			             = default;
    array_type(const array_type&)                    = delete;
    array_type(array_type&&)            	     = delete;
    auto operator=(const array_type&) -> array_type& = delete;
    auto operator=(array_type&&) -> array_type&      = delete;
    ~array_type()                                    = default;

    auto element() const noexcept -> type_id { return _element; }
    auto length() const noexcept -> std::size_t { return _length; }

    constexpr inline auto get()        const noexcept -> llvm::ArrayType* { return _type; }
    constexpr inline auto operator*()  const noexcept -> llvm::ArrayType* { return _type; }
    constexpr inline auto operator->() const noexcept -> llvm::ArrayType* { return _type; }
};

} // namespace type
