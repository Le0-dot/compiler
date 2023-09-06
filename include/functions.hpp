#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <functional>
#include <optional>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

#include "hash.hpp"
#include "type/registry.hpp"
#include "type/type_id.hpp"


class casts {
public:
    using cast_inserter = llvm::Value*(llvm::IRBuilderBase*, llvm::Value*);
    using inserter_wrapper = std::function<cast_inserter>;
    
private:
    std::unordered_map<type::type_id, inserter_wrapper> _casts{};

public:
    auto get(type::type_id to_type) const -> std::optional<inserter_wrapper>;
    void insert(type::type_id to_type);
    void specialize(type::type_id to_type, inserter_wrapper inserter);
};

class unary_operator {
public:
    using unary_inserter = llvm::Value*(llvm::IRBuilderBase*, llvm::Value*);
    using inserter_wrapper = std::function<unary_inserter>;
    using operator_info = std::pair<type::type_id, inserter_wrapper>;

    using variation = std::unordered_map<type::type_id, operator_info>;

private:
     
    std::uint64_t _precedense{};
    variation _unary{};

public:
    unary_operator() = default;
    inline explicit unary_operator(std::uint64_t precedense) : _precedense{precedense} {}

    inline auto precedense() const noexcept -> std::uint64_t { return _precedense; }

    auto get(type::type_id operand_type) -> operator_info;
    void insert(type::type_id operand_type, type::type_id return_type);
    void specialize(type::type_id operand_type, inserter_wrapper inserter);
};

class binary_operator {
public:
    using binary_inserter = llvm::Value*(llvm::IRBuilderBase*, llvm::Value*, llvm::Value*);
    using inserter_wrapper = std::function<binary_inserter>;
    using operator_info = std::pair<type::type_id, inserter_wrapper>; 

    using variation = std::unordered_map<std::pair<type::type_id, type::type_id>, operator_info, pair_hash<type::type_id, type::type_id>>;

private:
    std::uint64_t _precedense{};
    variation _binary{};

public:
    binary_operator() = default;
    inline explicit binary_operator(std::uint64_t precedense) : _precedense{precedense} {}

    inline auto precedense() const noexcept -> std::uint64_t { return _precedense; }

    auto get(type::type_id left, type::type_id right) -> operator_info;
    void insert(type::type_id left, type::type_id right, type::type_id return_type);
    void specialize(type::type_id left, type::type_id right, inserter_wrapper inserter);

    auto begin() const { return _binary.begin(); }
    auto begin()       { return _binary.begin(); }

    auto end() const { return _binary.end(); }
    auto end()       { return _binary.end(); }
};

class special_functions {
    std::unordered_map<type::type_id, casts> _casts{};
    std::unordered_map<std::string, unary_operator> _unary{};
    std::unordered_map<std::string, binary_operator> _binary{};

public:
    inline auto cast(type::type_id from_type) noexcept -> casts& { return _casts[from_type]; }
    inline auto unary(const std::string& oper) noexcept -> unary_operator& { return _unary[oper]; }
    inline auto binary(const std::string& oper) noexcept -> binary_operator& { return _binary[oper]; }

    auto new_unary(const std::string& oper, std::uint64_t precedense) noexcept -> bool;
    auto new_binary(const std::string& oper, std::uint64_t precedense) noexcept -> bool;
};
