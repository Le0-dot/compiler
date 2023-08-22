#pragma once

#include <utility>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type.hpp"
#include "type_id.hpp"


namespace type {

class function_type : public type {
    friend auto std::make_unique<function_type>(
	    llvm::FunctionType*&, 
	    const std::vector<::type::type_id>&, 
	    ::type::type_id&
	) -> std::unique_ptr<function_type>;

    std::vector<type_id> _params;
    type_id _return;

    function_type(llvm::FunctionType* ftype, std::vector<type_id>  params, type_id ret) noexcept
	: type{ftype}
	, _params{std::move(params)}
	, _return{ret}
    {}

public:
    auto params() const noexcept -> const auto& { return _params; }
    auto return_type() const noexcept -> type_id { return _return; }
};

} // namespace type
