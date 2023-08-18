#pragma once

#include <memory>
#include <vector>

#include <llvm/IR/Type.h>

#include "type_id.hpp"


namespace type {

class type {
    friend std::unique_ptr<type> std::make_unique<type>(llvm::Type*&&);
    friend std::unique_ptr<type> std::make_unique<type>(llvm::IntegerType*&&);

private:
    llvm::Type* _type{};

protected:
    type(llvm::Type* type) noexcept : _type{type} {}

public:
    type()			 = default;
    type(type&&)            	 = delete;
    type(const type&)            = delete;
    type& operator=(type&&)      = delete;
    type& operator=(const type&) = delete;
    ~type()                      = default;

    constexpr inline llvm::Type* get() const noexcept { return _type; }
    constexpr inline llvm::Type* operator*() const noexcept { return _type; }
    constexpr inline llvm::Type* operator->() const noexcept { return _type; }
};

}
