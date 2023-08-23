#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <functional>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

#include "hash.hpp"
#include "type/type_id.hpp"


class special_functions {
public:
    using cast_function_inserter = llvm::Value*(llvm::IRBuilderBase*, llvm::Value*);
    using unary_function_inserter = llvm::Value*(llvm::IRBuilderBase*, llvm::Value*);
    using binary_function_inserter = llvm::Value*(llvm::IRBuilderBase*, llvm::Value*, llvm::Value*);

    using cast_inserter_wrapper = std::function<cast_function_inserter>;
    using unary_inserter_wrapper = std::function<unary_function_inserter>;
    using binary_inserter_wrapper = std::function<binary_function_inserter>;

private:
    using unary_variation = std::unordered_map<type::type_id, unary_inserter_wrapper>;
    using binary_variation = std::unordered_map<std::pair<type::type_id, type::type_id>, binary_inserter_wrapper, pair_hash<type::type_id, type::type_id>>;

    std::unordered_map<std::pair<type::type_id, type::type_id>, cast_inserter_wrapper, pair_hash<type::type_id, type::type_id>> _casts{};
    std::unordered_map<std::string, unary_variation> _unary{};
    std::unordered_map<std::string, binary_variation> _binary{};

    static constexpr auto dummy_cast = [] (llvm::IRBuilderBase*, llvm::Value*) -> llvm::Value* { return nullptr; };
    static constexpr auto dummy_unary = [] (llvm::IRBuilderBase*, llvm::Value*) -> llvm::Value* { return nullptr; };
    static constexpr auto dummy_binary = [] (llvm::IRBuilderBase*, llvm::Value*, llvm::Value*) -> llvm::Value* { return nullptr; };

public:
    auto cast_exists(type::type_id from_t, type::type_id to_t) const noexcept -> bool;
    auto cast(type::type_id from_t, type::type_id to_t) noexcept -> cast_inserter_wrapper;
    void insert_dummy_cast(type::type_id from_t, type::type_id to_t) noexcept;
    void insert_cast(type::type_id from_t, type::type_id to_t, cast_inserter_wrapper inserter) noexcept;

    auto unary_exists(const std::string& oper, type::type_id tid) const noexcept -> bool;
    auto unary(const std::string& oper, type::type_id tid) noexcept -> unary_inserter_wrapper;
    void insert_dummy_unary(const std::string& oper, type::type_id tid) noexcept;
    void insert_unary(const std::string& oper, type::type_id tid, unary_inserter_wrapper inserter) noexcept;

    auto binary_exists(const std::string& oper, type::type_id left, type::type_id right) const noexcept -> bool;
    auto binary(const std::string& oper, type::type_id left, type::type_id right) noexcept -> binary_inserter_wrapper;
    void insert_dummy_binary(const std::string& oper, type::type_id left, type::type_id right) noexcept;
    void insert_binary(const std::string& oper, type::type_id left, type::type_id right, binary_inserter_wrapper inserter) noexcept;
};
