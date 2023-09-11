#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type.hpp"


namespace type {

class struct_type : public type {
public:
    using members_type = std::vector<std::pair<std::string, type_id>>;

private:
    members_type _members;

public:
    struct_type(llvm::StructType* stype, std::vector<std::pair<std::string, type_id>>  members)
	: type{stype}
	, _members{std::move(members)}
    {}

    auto members() const noexcept -> const auto& { return _members; }

    constexpr inline auto get()        const noexcept -> llvm::StructType* { return static_cast<llvm::StructType*>(type::get()); }
    constexpr inline auto operator*()  const noexcept -> llvm::StructType* { return static_cast<llvm::StructType*>(type::get()); }
    constexpr inline auto operator->() const noexcept -> llvm::StructType* { return static_cast<llvm::StructType*>(type::get()); }
};

} // namespace type
