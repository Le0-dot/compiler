#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include <llvm/IR/LLVMContext.h>

#include "type_id.hpp"
#include "type.hpp"
#include "array_type.hpp"
#include "anon_struct_type.hpp"
#include "struct_type.hpp"
#include "function_type.hpp"
#include "../hash.hpp"


namespace type {

class registry {
    using anon_struct_key = std::vector<type_id>;
    using array_key       = std::pair<type_id, std::size_t>;
    using function_key    = std::pair<std::vector<type_id>, type_id>;

    using anon_struct_hash = hash_for_t<anon_struct_key>;
    using array_hash       = hash_for_t<array_key>;
    using function_hash    = hash_for_t<function_key>;

    llvm::LLVMContext* _context;

    std::unordered_map<std::string, type_id> _names;
    std::unordered_map<anon_struct_key, type_id, anon_struct_hash> _anon_struct_ids;
    std::unordered_map<array_key, type_id, array_hash> _array_ids;
    std::unordered_map<function_key, type_id, function_hash> _function_ids;

    std::unordered_map<type_id, type> _primitives;
    std::unordered_map<type_id, function_type> _functions;
    std::unordered_map<type_id, array_type> _arrays;
    std::unordered_map<type_id, struct_type> _structs;
    std::unordered_map<type_id, anon_struct_type> _anon_structs;

public:
    registry()                                   = delete;
    registry(const registry&)		         = delete;
    registry(registry&&)      		         = delete;
    auto operator=(const registry&) -> registry& = delete;
    auto operator=(registry&&) -> registry&      = delete;
    ~registry()				         = default;

    explicit registry(llvm::LLVMContext* context) : _context{context} {
	make_primitives();
    }

    [[nodiscard]] auto id(const std::string& name) const noexcept -> type_id;

    [[nodiscard]] auto id(const std::vector<type_id>& members)             noexcept -> type_id;
    [[nodiscard]] auto id(const std::vector<type_id>& params, type_id ret) noexcept -> type_id;
    [[nodiscard]] auto id(type_id elements, std::size_t size)              noexcept -> type_id;

    [[nodiscard]] auto get(type_id tid)             const noexcept -> const type*;
    [[nodiscard]] auto get_struct(type_id tid)	    const noexcept -> const struct_type*;
    [[nodiscard]] auto get_anon_struct(type_id tid) const noexcept -> const anon_struct_type*;
    [[nodiscard]] auto get_function(type_id tid)    const noexcept -> const function_type*;
    [[nodiscard]] auto get_array(type_id tid)       const noexcept -> const array_type*;

    [[nodiscard]] auto get(const std::string& name)          const noexcept -> const type*;
    [[nodiscard]] auto get_struct(const std::string& name)   const noexcept -> const struct_type*;
    [[nodiscard]] auto get_function(const std::string& name) const noexcept -> const function_type*;
    [[nodiscard]] auto get_array(const std::string& name)    const noexcept -> const array_type*;

    [[nodiscard]] auto make_struct(const std::string& name, const struct_type::members_type& members)          noexcept -> const struct_type&;
    [[nodiscard]] auto make_anon_struct(const std::vector<type_id>& members)          noexcept -> const anon_struct_type&;
    [[nodiscard]] auto make_function(const std::vector<type_id>& params, type_id ret) noexcept -> const function_type&;
    [[nodiscard]] auto make_array(type_id elements, std::size_t size)                 noexcept -> const array_type&;

    void make_alias(const std::string& alias, type_id tid) noexcept { _names[alias] = tid; }

private:
    static auto next_id() noexcept -> type_id {
	static type_id tid{type_id::primitive_bound};
	return ++tid;
    }

    void make_primitives() noexcept;
};

} // namespace type
