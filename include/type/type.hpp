#pragma once

#include <memory>
#include <vector>

#include <llvm/IR/Type.h>

#include "type_id.hpp"


namespace type {

class type {
    friend auto std::make_unique<type>(llvm::Type*&&) -> std::unique_ptr<type>;
    friend auto std::make_unique<type>(llvm::IntegerType*&&) -> std::unique_ptr<type>;

    llvm::Type* _type{};

protected:
    explicit type(llvm::Type* type) noexcept : _type{type} {}

public:
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
