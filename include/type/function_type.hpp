#pragma once

#include <utility>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type_id.hpp"


namespace type {

class function_type {
    llvm::FunctionType* _type{};
    std::vector<type_id> _params{};
    type_id _return{};

public:
    function_type(llvm::FunctionType* type, std::vector<type_id>  params, type_id ret) noexcept
	: _type{type}
	, _params{std::move(params)}
	, _return{ret}
    {}

    function_type()			                   = default;
    function_type(const function_type&)                    = delete;
    function_type(function_type&&)            	           = delete;
    auto operator=(const function_type&) -> function_type& = delete;
    auto operator=(function_type&&) -> function_type&      = delete;
    ~function_type()                                       = default;

    auto params() const noexcept -> const auto& { return _params; }
    auto return_type() const noexcept -> type_id { return _return; }

    constexpr inline auto get()        const noexcept -> llvm::FunctionType* { return _type; }
    constexpr inline auto operator*()  const noexcept -> llvm::FunctionType* { return _type; }
    constexpr inline auto operator->() const noexcept -> llvm::FunctionType* { return _type; }
};

} // namespace type
