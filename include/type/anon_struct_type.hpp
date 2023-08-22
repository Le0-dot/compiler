#pragma once

#include <memory>
#include <vector>
#include <utility>

#include <llvm/IR/DerivedTypes.h>

#include "type.hpp"


namespace type {

class anon_struct_type : public type {
    friend std::unique_ptr<anon_struct_type> std::make_unique<anon_struct_type>(llvm::StructType*&, const std::vector<::type::type_id>&);

private:
    std::vector<type_id> _members;

    anon_struct_type(llvm::StructType* _type, const std::vector<type_id>& members)
	: type{_type}
	, _members{members}
    {}

public:
    auto members() const noexcept -> const auto& { return _members; }
};

}
