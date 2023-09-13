#pragma once

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include <any_tree.hpp>

#include "tree.hpp"
#include "scope.hpp"
#include "functions.hpp"
#include "type/type_id.hpp"
#include "type/registry.hpp"


class code_generator {
public:
    using visitor = any_tree::const_children_visitor<llvm::Value*>;

private:
    llvm::LLVMContext* _context;
    llvm::IRBuilder<> _builder;
    llvm::Module _module;

    scope_manager<llvm::AllocaInst*, llvm::Function*> _scope{};
    scope<llvm::Function*, void> _functions{};
    special_functions* _special;
    type::registry* _types;

    visitor _visitor;

    auto file            (const visitor& visitor, const file_node& node)             -> llvm::Value*;
    auto function        (const visitor& visitor, const function_node& node)         -> llvm::Value*;
    auto return_statement(const visitor& visitor, const return_statement_node& node) -> llvm::Value*;
    auto let_statement   (const visitor& visitor, const let_statement_node& node)    -> llvm::Value*;
    auto var_def         (const visitor& visitor, const var_def_node& node)          -> llvm::Value*;
    auto binary_expr     (const visitor& visitor, const binary_expr_node& node)      -> llvm::Value*;
    auto call            (const visitor& visitor, const call_node& node)             -> llvm::Value*;
    auto implicit_cast   (const visitor& visitor, const implicit_cast_node& node)    -> llvm::Value*;

    auto identifier(const identifier_node& node) -> llvm::Value*;

    auto integer_literal (const integer_literal_node& node)  -> llvm::Value*;
    auto floating_literal(const floating_literal_node& node) -> llvm::Value*;
    auto char_literal    (const char_literal_node& node)     -> llvm::Value*;
    auto string_literal  (const string_literal_node& node)   -> llvm::Value*;
    auto bool_literal    (const bool_literal_node& node)     -> llvm::Value*;

public:
    code_generator(const std::string& module_name, llvm::LLVMContext* context, special_functions* special, type::registry* types);

    auto get_visitor() -> visitor& { return _visitor; }
    auto get_module() -> llvm::Module& { return _module; };
};
