#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <functional>
#include <optional>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

#include "hash.hpp"
#include "type/type_id.hpp"


class casts {
public:
    using cast_inserter = llvm::Value*(llvm::IRBuilderBase*, llvm::Value*);
    using inserter_wrapper = std::function<cast_inserter>;
    
private:
    std::unordered_map<type::type_id, inserter_wrapper> _casts{};

public:
    auto get(type::type_id to_type) const -> std::optional<inserter_wrapper>;
    void insert(type::type_id to_type, inserter_wrapper inserter);
};

class unary_operator {
public:
    using unary_inserter = llvm::Value*(llvm::IRBuilderBase*, llvm::Value*);
    using inserter_wrapper = std::function<unary_inserter>;

private:
    using variation = std::unordered_map<type::type_id, inserter_wrapper>;
     
    std::uint64_t _precedense{};
    variation _unary{};

public:
    inline explicit unary_operator(std::uint64_t precedense) : _precedense{precedense} {}

    inline auto precedense() const noexcept -> std::uint64_t { return _precedense; }

    auto get(type::type_id operand_type) -> std::optional<inserter_wrapper>;
    void insert(type::type_id operand_type, inserter_wrapper inserter);
};

class binary_operator {
public:
    using binary_inserter = llvm::Value*(llvm::IRBuilderBase*, llvm::Value*, llvm::Value*);
    using inserter_wrapper = std::function<binary_inserter>;

private:
    using variation = std::unordered_map<std::pair<type::type_id, type::type_id>, inserter_wrapper, pair_hash<type::type_id, type::type_id>>;
     
    std::uint64_t _precedense{};
    variation _binary{};

public:
    inline explicit binary_operator(std::uint64_t precedense) : _precedense{precedense} {}

    inline auto precedense() const noexcept -> std::uint64_t { return _precedense; }

    auto get(type::type_id left, type::type_id right) -> std::optional<inserter_wrapper>;
    void insert(type::type_id left, type::type_id right, inserter_wrapper inserter);
};

class special_functions {
    std::unordered_map<type::type_id, casts> _casts{};
    std::unordered_map<std::string, unary_operator> _unary{};
    std::unordered_map<std::string, binary_operator> _binary{};

public:
    inline auto casts(type::type_id from_type) noexcept -> casts& { return _casts[from_type]; }
    inline auto unary(const std::string& oper) noexcept -> unary_operator& { return _unary[oper]; }
    inline auto binary(const std::string& oper) noexcept -> binary_operator& { return _binary[oper]; }

    auto new_unary(const std::string& oper, std::uint64_t precedense) noexcept -> bool;
    auto new_binary(const std::string& oper, std::uint64_t precedense) noexcept -> bool;
};
