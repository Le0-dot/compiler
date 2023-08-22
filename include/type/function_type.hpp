#pragma once

#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type.hpp"
#include "type_id.hpp"


namespace type {

class function_type : public type {
    friend std::unique_ptr<function_type> std::make_unique<function_type>(
	    llvm::FunctionType*&, const std::vector<::type::type_id>&, ::type::type_id&);

private:
    std::vector<type_id> _params;
    type_id _return;

    function_type(llvm::FunctionType* _type, const std::vector<type_id>& params, type_id ret) noexcept
	: type{_type}
	, _params{params}
	, _return{std::move(ret)}
    {}

public:
    auto params() const noexcept -> const auto& { return _params; }
    auto return_type() const noexcept -> type_id { return _return; }
};

}
