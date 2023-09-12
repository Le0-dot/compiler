#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type_id.hpp"


namespace type {

class anon_struct_type {
    llvm::StructType* _type{};
    std::vector<type_id> _members{};

public:
    anon_struct_type(llvm::StructType* type, std::vector<type_id>  members)
	: _type{type}
	, _members{std::move(members)}
    {}

    anon_struct_type()			                         = default;
    anon_struct_type(const anon_struct_type&)                    = delete;
    anon_struct_type(anon_struct_type&&)            	         = delete;
    auto operator=(const anon_struct_type&) -> anon_struct_type& = delete;
    auto operator=(anon_struct_type&&) -> anon_struct_type&      = delete;
    ~anon_struct_type()                                          = default;

    auto members() const noexcept -> const auto& { return _members; }

    constexpr inline auto get()        const noexcept -> llvm::StructType* { return _type; }
    constexpr inline auto operator*()  const noexcept -> llvm::StructType* { return _type; }
    constexpr inline auto operator->() const noexcept -> llvm::StructType* { return _type; }
};

} // namespace type
