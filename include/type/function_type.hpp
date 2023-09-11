#pragma once

#include <utility>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type.hpp"
#include "type_id.hpp"


namespace type {

class function_type : public type {
    std::vector<type_id> _params;
    type_id _return;

public:
    function_type(llvm::FunctionType* ftype, std::vector<type_id>  params, type_id ret) noexcept
	: type{ftype}
	, _params{std::move(params)}
	, _return{ret}
    {}

    auto params() const noexcept -> const auto& { return _params; }
    auto return_type() const noexcept -> type_id { return _return; }

    constexpr inline auto get()        const noexcept -> llvm::FunctionType* { return static_cast<llvm::FunctionType*>(type::get()); }
    constexpr inline auto operator*()  const noexcept -> llvm::FunctionType* { return static_cast<llvm::FunctionType*>(type::get()); }
    constexpr inline auto operator->() const noexcept -> llvm::FunctionType* { return static_cast<llvm::FunctionType*>(type::get()); }
};

} // namespace type
