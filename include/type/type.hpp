#pragma once

#include <memory>
#include <vector>

#include <llvm/IR/Type.h>

#include "type_id.hpp"


namespace type {

class type {
    llvm::Type* _type{};

public:
    explicit type(llvm::Type* type) noexcept : _type{type} {}

    type()			         = default;
    type(type&&)            	         = delete;
    type(const type&)                    = delete;
    auto operator=(type&&) -> type&      = delete;
    auto operator=(const type&) -> type& = delete;
    ~type()                              = default;

    constexpr inline auto get()        const noexcept -> llvm::Type* { return _type; }
    constexpr inline auto operator*()  const noexcept -> llvm::Type* { return _type; }
    constexpr inline auto operator->() const noexcept -> llvm::Type* { return _type; }
};

} // namespace type
