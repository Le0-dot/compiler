#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type.hpp"


namespace type {

class anon_struct_type : public type {
    friend auto std::make_unique<anon_struct_type>(
	    llvm::StructType*&,
	    const std::vector<::type::type_id>&
	) -> std::unique_ptr<anon_struct_type>;

    std::vector<type_id> _members;

    anon_struct_type(llvm::StructType* atype, std::vector<type_id>  members)
	: type{atype}
	, _members{std::move(members)}
    {}

public:
    auto members() const noexcept -> const auto& { return _members; }

    constexpr inline auto get()        const noexcept -> llvm::StructType* { return static_cast<llvm::StructType*>(type::get()); }
    constexpr inline auto operator*()  const noexcept -> llvm::StructType* { return static_cast<llvm::StructType*>(type::get()); }
    constexpr inline auto operator->() const noexcept -> llvm::StructType* { return static_cast<llvm::StructType*>(type::get()); }
};

} // namespace type
