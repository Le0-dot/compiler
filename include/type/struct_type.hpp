#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type_id.hpp"


namespace type {

class struct_type {
public:
    using members_type = std::vector<std::pair<std::string, type_id>>;

private:
    llvm::StructType* _type{};
    members_type _members{};

public:
    struct_type(llvm::StructType* type, std::vector<std::pair<std::string, type_id>> members)
	: _type{type}
	, _members{std::move(members)}
    {}

    struct_type()			               = default;
    struct_type(const struct_type&)                    = delete;
    struct_type(struct_type&&)            	       = delete;
    auto operator=(const struct_type&) -> struct_type& = delete;
    auto operator=(struct_type&&) -> struct_type&      = delete;
    ~struct_type()                                     = default;

    auto members() const noexcept -> const auto& { return _members; }

    constexpr inline auto get()        const noexcept -> llvm::StructType* { return _type; }
    constexpr inline auto operator*()  const noexcept -> llvm::StructType* { return _type; }
    constexpr inline auto operator->() const noexcept -> llvm::StructType* { return _type; }

    inline auto get_ptr() const noexcept -> llvm::Type* { return _type->getPointerTo(); }
};

} // namespace type
