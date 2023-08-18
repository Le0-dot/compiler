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

private:
    llvm::LLVMContext& _context;

    std::unordered_map<type_id, std::unique_ptr<type>> _ids;
    std::unordered_map<std::string, type_id> _names;

    std::unordered_map<anon_struct_key, type_id, anon_struct_hash> _anon_structs;
    std::unordered_map<array_key, type_id, array_hash> _arrays;
    std::unordered_map<function_key, type_id, function_hash> _functions;

public:
    registry()                           = delete;
    registry(const registry&)		 = delete;
    registry(registry&&)      		 = delete;
    registry& operator=(const registry&) = delete;
    registry& operator=(registry&&)      = delete;
    ~registry()				 = default;

    registry(llvm::LLVMContext& context) : _context{context} {
	make_primitives();
    }

    [[nodiscard]] auto id(const std::string& name) noexcept -> type_id { return _names[name]; }

    [[nodiscard]] auto id(const std::vector<type_id>& members)             noexcept -> type_id;
    [[nodiscard]] auto id(const std::vector<type_id>& params, type_id ret) noexcept -> type_id;
    [[nodiscard]] auto id(type_id elements, std::size_t size)              noexcept -> type_id;

    [[nodiscard]] auto get(type_id id)	            -> type& { return *_ids[id]; }
    [[nodiscard]] auto get(const std::string& name) -> type& { return get(id(name)); }

    [[nodiscard]] auto get_struct(const std::string& name)			     noexcept -> struct_type&;
    [[nodiscard]] auto get_anon_struct(const std::vector<type_id>& members)          noexcept -> anon_struct_type&;
    [[nodiscard]] auto get_function(const std::vector<type_id>& params, type_id ret) noexcept -> function_type&;
    [[nodiscard]] auto get_array(type_id elements, std::size_t size)                 noexcept -> array_type&;

    auto make_struct(const std::string& name, const std::vector<std::pair<std::string, type_id>>& members) noexcept -> type_id;

    void make_alias(const std::string& alias, type_id id) noexcept { _names[alias] = id; }

    [[nodiscard]] auto is_struct(type_id id)                       noexcept -> bool;
    [[nodiscard]] auto is_array(type_id id)                        noexcept -> bool;
    [[nodiscard]] auto is_function(type_id id)                     noexcept -> bool;

private:
    static auto next_id() noexcept -> type_id {
	static type_id _id{type_id::primitive_bound};
	return ++_id;
    }

    void make_primitives() noexcept;
};

}
