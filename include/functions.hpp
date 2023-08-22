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
    using binary_variation = std::unordered_map<std::pair<type::type_id, type::type_id>, binary_inserter_wrapper>;

    std::unordered_map<std::pair<type::type_id, type::type_id>, cast_inserter_wrapper, pair_hash<type::type_id, type::type_id>> _casts{};
    std::unordered_map<std::string, unary_variation> _unary{};
    std::unordered_map<std::string, binary_variation> _binary{};

public:
    auto cast_exists(type::type_id from_t, type::type_id to_t) -> bool;
    auto cast(type::type_id from_t, type::type_id to_t) -> cast_inserter_wrapper;
    void insert_cast(type::type_id from_t, type::type_id to_t, cast_inserter_wrapper);

    auto unary_exists(const std::string& oper, type::type_id tid) -> bool;
    auto unary(const std::string& oper, type::type_id tid) -> unary_inserter_wrapper;
    void insert_unary(const std::string& oper, type::type_id tid, unary_inserter_wrapper);

    auto binary_exists(const std::string& oper, type::type_id left, type::type_id right) -> bool;
    auto binary(const std::string& oper, type::type_id left, type::type_id right) -> binary_inserter_wrapper;
    void insert_binary(const std::string& oper, type::type_id left, type::type_id right, binary_inserter_wrapper);
};
