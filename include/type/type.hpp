#pragma once

#include <memory>
#include <vector>

#include <llvm/IR/Type.h>

#include "type/anon_struct_type.hpp"
#include "type/array_type.hpp"
#include "type/function_type.hpp"
#include "type/struct_type.hpp"
#include "type_id.hpp"


namespace type {

class type {
    llvm::Type* _type{};

public:
    explicit type(llvm::Type* type)             noexcept : _type{type} {}
    explicit type(const array_type& type)       noexcept : _type{type.get()} {}
    explicit type(const function_type& type)    noexcept : _type{type.get()} {}
    explicit type(const struct_type& type)      noexcept : _type{type.get()} {}
    explicit type(const anon_struct_type& type) noexcept : _type{type.get()} {}

    type()			         = default;
    type(const type&)                    = default;
    type(type&&)            	         = default;
    auto operator=(const type&) -> type& = default;
    auto operator=(type&&) -> type&      = default;
    ~type()                              = default;

    constexpr inline auto get()        const noexcept -> llvm::Type* { return _type; }
    constexpr inline auto operator*()  const noexcept -> llvm::Type* { return _type; }
    constexpr inline auto operator->() const noexcept -> llvm::Type* { return _type; }
};

} // namespace type
