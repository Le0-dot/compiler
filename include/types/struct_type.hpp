#pragma once

#include <memory>
#include <vector>
#include <utility>

#include <llvm/IR/DerivedTypes.h>

#include "type.hpp"


namespace type {

class struct_type : public type {
    friend std::unique_ptr<struct_type> std::make_unique<struct_type>(
	    llvm::StructType*&, const std::vector<std::pair<std::string, ::type::type_id>>&);

private:
    std::vector<std::pair<std::string, type_id>> _members;

    struct_type(llvm::StructType* _type, const std::vector<std::pair<std::string, type_id>>& members)
	: type{_type}
	, _members{members}
    {}

public:
    auto members() const noexcept -> const auto& { return _members; }
};

}
