#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type.hpp"


namespace type {

class struct_type : public type {
    friend auto std::make_unique<struct_type>(
	    llvm::StructType*&, 
	    const std::vector<std::pair<std::string, 
	    ::type::type_id>>&
	) -> std::unique_ptr<struct_type>;

    std::vector<std::pair<std::string, type_id>> _members;

    struct_type(llvm::StructType* stype, std::vector<std::pair<std::string, type_id>>  members)
	: type{stype}
	, _members{std::move(members)}
    {}

public:
    auto members() const noexcept -> const auto& { return _members; }
};

} // namespace type
