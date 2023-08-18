#pragma once

#include <memory>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

#include "type.hpp"
#include "type_id.hpp"


namespace type {

class array_type : public type {
    friend std::unique_ptr<array_type> std::make_unique<array_type>(llvm::ArrayType*&, ::type::type_id&, std::size_t&);

private:
    type_id _element;
    std::size_t _length;

    array_type(llvm::ArrayType* _type, type_id element, std::size_t length) noexcept 
	: type{_type}
	, _element{std::move(element)}
	, _length{std::move(length)}
    {}

public:
    auto element() const noexcept -> type_id { return _element; }
    auto length() const noexcept -> std::size_t { return _length; }
};

}
