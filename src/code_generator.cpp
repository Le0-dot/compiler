#include <algorithm>

#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include "code_generator.hpp"
#include "tree.hpp"
#include "type/type_id.hpp"


auto code_generator::file(const visitor& visitor, const file_node& node) -> llvm::Value* {
    std::cerr << "file" << std::endl;

    std::vector<llvm::Value*> functions{};
    functions.reserve(node.children_size());

    std::ranges::transform(
	    node.children(), 
	    std::back_inserter(functions),
	    [&visitor] (const std::any& node) { return any_tree::visit_node(visitor, node); }
    );

    bool invalid_functions = std::ranges::any_of(
	    functions, 
	    [] (llvm::Value* value) { return value == nullptr; }
    );
    if(invalid_functions) {
	std::cerr << "invalid function argument" << std::endl;
	return nullptr;
    }

    return functions.back();
}

auto code_generator::function(const visitor& visitor, const function_node& node) -> llvm::Value* {
    std::cerr << "function" << std::endl;
    llvm::FunctionType* func_type = *_types->get_function(node.payload().params_type, node.payload().return_type);
    llvm::Function* func = llvm::Function::Create(
	    func_type, 
	    llvm::Function::ExternalLinkage,
	    node.payload().name,
	    _module
    );

    _scope.add(node.payload().name, func);
    scope_pusher pusher{&_scope};

    auto arg_iter = func->arg_begin();
    auto param_iter = node.payload().params.begin();
    while(arg_iter != func->arg_end()) {
	arg_iter->setName(*param_iter);
	_scope.add(*param_iter++, arg_iter++);
    }

    llvm::BasicBlock* block = llvm::BasicBlock::Create(*_context, "entry", func);
    _builder.SetInsertPoint(block);

    node.for_each_child([&visitor] (const std::any& node) { return any_tree::visit_node(visitor, node); });

    if(llvm::verifyFunction(*func)) {
	func->eraseFromParent();
	return nullptr;
    }

    return func;
}

auto code_generator::statement(const visitor& visitor, const statement_node& node) -> llvm::Value* {
    std::cerr << "statement" << std::endl;
    llvm::Value* inner = any_tree::visit_node(visitor, node.child_at(0));

    if(node.payload().is_return) {
	return _builder.CreateRet(inner);
    } 

    return inner;
}

auto code_generator::binary_expr(const visitor& visitor, const binary_expr_node& node) -> llvm::Value* {
    std::cerr << "binary" << std::endl;
    llvm::Value* lhs = any_tree::visit_node(visitor, node.child_at(0));
    if(lhs == nullptr) {
	std::cerr << "invalid lhs expression" << std::endl;
	return nullptr;
    }

    llvm::Value* rhs = any_tree::visit_node(visitor, node.child_at(1));
    if(rhs == nullptr) {
	std::cerr << "invalid rhs expression" << std::endl;
	return nullptr;
    }

    auto bin_operator = _special->binary(node.payload().oper).get(node.payload().lhs, node.payload().rhs);
    if(!type::valid(bin_operator.return_type)) {
	std::cerr << "invalid operator return type" << std::endl;
	return nullptr;
    }

    // pointer to function is invalid
    if(!bin_operator.inserter) {
	std::cerr << "invalid operator inserter function" << std::endl;
	return nullptr;
    }

    return bin_operator.inserter(&_builder, lhs, rhs);
}

auto code_generator::call(const visitor& visitor, const call_node& node) -> llvm::Value* {
    std::cerr << "call" << std::endl;
    llvm::Function* callee = _module.getFunction(node.payload().callee);
    if(callee == nullptr) {
	return nullptr;
    }

    std::vector<llvm::Value*> param_values{};
    param_values.reserve(callee->arg_size());

    std::ranges::transform(
	    node.children(), 
	    std::back_inserter(param_values),
	    [&visitor] (const std::any& node) { return any_tree::visit_node(visitor, node); }
    );

    bool invalid_params = std::ranges::any_of(
	    param_values, 
	    [] (llvm::Value* value) { return value == nullptr; }
    );
    if(invalid_params) {
	std::cerr << "invalid function argument" << std::endl;
	return nullptr;
    }

    return _builder.CreateCall(callee, param_values, "call");
}

auto code_generator::implicit_cast(const visitor& visitor, const implicit_cast_node& node) -> llvm::Value* {
    std::cerr << "cast" << std::endl;
    auto cast = _special->cast(node.payload().from_type).get(node.payload().to_type);
    if(!cast.has_value()) {
	return nullptr;
    }
    
    llvm::Value* inner = any_tree::visit_node(visitor, node.child_at(0));
    if(inner == nullptr) {
	return nullptr;
    }

    return cast.value()(&_builder, inner);
}

auto code_generator::identifier(const identifier_node& node) -> llvm::Value* {
    std::cerr << "identifier" << std::endl;
    return _scope.get(node.payload()).value_or(nullptr);
}

auto code_generator::integer_literal(const integer_literal_node& node) -> llvm::Value* {
    std::cerr << "integer_literal" << std::endl;
    return llvm::ConstantInt::get(*_types->get(node.payload().type), node.payload().value);
}

auto code_generator::floating_literal(const floating_literal_node& node) -> llvm::Value* {
    std::cerr << "floating_literal" << std::endl;
    return llvm::ConstantFP::get(*_types->get(node.payload().type), node.payload().value);
}

auto code_generator::char_literal(const char_literal_node& node) -> llvm::Value* {
    std::cerr << "char_literal" << std::endl;
    return llvm::ConstantInt::get(*_types->get(node.payload().type), node.payload().value);
}

auto code_generator::string_literal(const string_literal_node& node) -> llvm::Value* {
}

auto code_generator::bool_literal(const bool_literal_node& node) -> llvm::Value* {
    std::cerr << "bool_literal" << std::endl;
    return llvm::ConstantInt::get(*_types->get(node.payload().type), static_cast<uint64_t>(node.payload().value));
}

code_generator::code_generator(const std::string& module_name, llvm::LLVMContext* context, special_functions* special, type::registry* types) 
    : _context{context}
    , _builder{*context}
    , _module{module_name, *context}
    , _special{special}
    , _types{types}
{
    _visitor = {
	any_tree::make_const_child_visitor<file_node>            ([this] (const file_node& node)              { return file(_visitor, node); }),
	any_tree::make_const_child_visitor<function_node>        ([this] (const function_node& node)          { return function(_visitor, node); }),
	any_tree::make_const_child_visitor<statement_node>       ([this] (const statement_node& node)         { return statement(_visitor, node); }),
	any_tree::make_const_child_visitor<binary_expr_node>     ([this] (const binary_expr_node& node)       { return binary_expr(_visitor, node); }),
	any_tree::make_const_child_visitor<call_node>            ([this] (const call_node& node)              { return call(_visitor, node); }),
	any_tree::make_const_child_visitor<implicit_cast_node>   ([this] (const implicit_cast_node& node)     { return implicit_cast(_visitor, node); }),
	any_tree::make_const_child_visitor<identifier_node>      ([this] (const identifier_node& node)        { return identifier(node); }),
	any_tree::make_const_child_visitor<integer_literal_node> ([this] (const integer_literal_node& node)   { return integer_literal(node); }),
	any_tree::make_const_child_visitor<floating_literal_node>([this] (const floating_literal_node& node)  { return floating_literal(node); }),
	any_tree::make_const_child_visitor<char_literal_node>    ([this] (const char_literal_node& node)      { return char_literal(node); }),
	any_tree::make_const_child_visitor<string_literal_node>  ([this] (const string_literal_node& node)    { return string_literal(node); }),
	any_tree::make_const_child_visitor<bool_literal_node>    ([this] (const bool_literal_node& node)      { return bool_literal(node); }),
    };
}
