#include <algorithm>
#include <iostream>

#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include "any_tree/visitor.hpp"
#include "code_generator.hpp"
#include "tree.hpp"
#include "type/type.hpp"
#include "type/type_id.hpp"


auto entry_builder(llvm::Function* func) -> llvm::IRBuilder<> {
    return llvm::IRBuilder<>{&func->getEntryBlock(), func->getEntryBlock().begin()};
}

auto code_generator::file(const visitor& visitor, const file_node& node) -> llvm::Value* {
    std::cout << "file" << std::endl;

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
	std::cout << "invalid function" << std::endl;
	return nullptr;
    }

    return functions.back();
}

auto code_generator::function(const visitor& visitor, const function_node& node) -> llvm::Value* {
    std::cout << "function" << std::endl;
    llvm::FunctionType* func_type = *_types->make_function(node.payload().params_type, node.payload().return_type);
    llvm::Function* func = llvm::Function::Create(
	    func_type, 
	    llvm::Function::ExternalLinkage,
	    node.payload().name,
	    _module
    );

    auto param = node.payload().params.begin();
    for(llvm::Argument& arg: func->args()) {
	arg.setName(*param++);
    }

    _functions.add(node.payload().name, func);
    scope_pusher pusher{&_scope, func};

    llvm::BasicBlock* block = llvm::BasicBlock::Create(*_context, "entry", func);
    _builder.SetInsertPoint(block);

    for(auto& arg: func->args()) {
	llvm::AllocaInst* inst = _builder.CreateAlloca(arg.getType(), nullptr, arg.getName());

	_builder.CreateStore(&arg, inst);

	_scope.add(std::string{arg.getName()}, inst);
    }

    llvm::Value* block_result = any_tree::visit_node(visitor, node.child_at(0));
    if(block_result == nullptr) {
	return nullptr;
    }

    if(llvm::verifyFunction(*func, &llvm::errs())) {
	func->eraseFromParent();
	return nullptr;
    }

    return func;
}

auto code_generator::return_statement(const visitor& visitor, const return_statement_node& node) -> llvm::Value* {
    std::cout << "return statement" << std::endl;
    return _builder.CreateRet(any_tree::visit_node(visitor, node.child_at(0)));
}

auto code_generator::let_statement(const visitor& visitor, const let_statement_node& node) -> llvm::Value* {
    std::cout << "let statement" << std::endl;

    std::vector<llvm::Value*> definitions{};
    definitions.reserve(node.children_size());

    std::ranges::transform(
	    node.children(), 
	    std::back_inserter(definitions),
	    [&visitor] (const std::any& node) { return any_tree::visit_node(visitor, node); }
    );

    bool invalid_definitions = std::ranges::any_of(
	    definitions, 
	    [] (llvm::Value* value) { return value == nullptr; }
    );
    if(invalid_definitions) {
	return nullptr;
    }

    return definitions.back();
}

auto code_generator::var_def(const visitor& visitor, const var_def_node& node) -> llvm::Value* {
    std::cout << "variable definition" << std::endl;

    llvm::Type* type = *_types->get(node.payload().type).value_or(type::type{});

    llvm::AllocaInst* inst = entry_builder(_scope.function()).CreateAlloca(type, nullptr, node.payload().name);

    if(node.children().empty()) {
	_scope.add(node.payload().name, inst);
	return inst;
    }

    llvm::Value* value = any_tree::visit_node(visitor, node.child_at(0));

    if(value == nullptr) {
	return nullptr;
    }

    _builder.CreateStore(value, inst);

    _scope.add(node.payload().name, inst);
    return inst;
}

auto code_generator::binary_expr(const visitor& visitor, const binary_expr_node& node) -> llvm::Value* {
    std::cout << "binary" << std::endl;
    llvm::Value* lhs = any_tree::visit_node(visitor, node.child_at(0));
    if(lhs == nullptr) {
	std::cout << "invalid lhs expression" << std::endl;
	return nullptr;
    }

    llvm::Value* rhs = any_tree::visit_node(visitor, node.child_at(1));
    if(rhs == nullptr) {
	std::cout << "invalid rhs expression" << std::endl;
	return nullptr;
    }

    if(node.payload().oper == "=") {
	if(auto *load = dyn_cast<llvm::LoadInst>(lhs); load != nullptr) {
	    lhs = load->getPointerOperand();
	    //load->removeFromParent();
	}
	return _builder.CreateStore(rhs, lhs);
    }

    auto bin_operator = _special->binary(node.payload().oper).get(node.payload().lhs, node.payload().rhs);
    if(!type::valid(bin_operator.return_type)) {
	std::cout << "invalid operator return type" << std::endl;
	return nullptr;
    }

    // pointer to function is invalid
    if(!bin_operator.inserter) {
	std::cout << "invalid operator inserter function" << std::endl;
	return nullptr;
    }

    return bin_operator.inserter(&_builder, lhs, rhs);
}

auto code_generator::if_stmt(const visitor& visitor, const if_node& node) -> llvm::Value* {
    std::cout << "if_stmt" << std::endl;
    if(node.payload().has_let && any_tree::visit_node(visitor, node.child_at(0)) == nullptr) {
	return nullptr;
    }

    llvm::Value* cond = any_tree::visit_node(visitor, node.child_at(1));
    if(cond == nullptr) {
	return nullptr;
    }

    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(*_context, "then", _scope.function());
    llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(*_context, "if_merge", _scope.function());

    _builder.CreateCondBr(cond, then_block, merge_block);

    // then
    _builder.SetInsertPoint(then_block);

    llvm::Value* then_value = any_tree::visit_node(visitor, node.child_at(2));
    if(then_value == nullptr) {
	return nullptr;
    }

    _builder.CreateBr(merge_block);

    // merge
    _builder.SetInsertPoint(merge_block);

    return then_value;
}

auto code_generator::if_else_stmt(const visitor& visitor, const if_else_node& node) -> llvm::Value* {
    std::cout << "if_else_stmt" << std::endl;
    if(node.payload().has_let && any_tree::visit_node(visitor, node.child_at(0)) == nullptr) {
	return nullptr;
    }

    llvm::Value* cond = any_tree::visit_node(visitor, node.child_at(1));
    if(cond == nullptr) {
	return nullptr;
    }

    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(*_context, "then", _scope.function());
    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(*_context, "else", _scope.function());
    llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(*_context, "if_merge", _scope.function());

    _builder.CreateCondBr(cond, then_block, else_block);

    // then
    _builder.SetInsertPoint(then_block);

    llvm::Value* then_value = any_tree::visit_node(visitor, node.child_at(2));
    if(then_value == nullptr) {
	return nullptr;
    }

    _builder.CreateBr(merge_block);
    then_block = _builder.GetInsertBlock();

    // else
    _builder.SetInsertPoint(else_block);

    llvm::Value* else_value = any_tree::visit_node(visitor, node.child_at(3));
    if(else_value == nullptr) {
	return nullptr;
    }

    _builder.CreateBr(merge_block);

    // merge
    _builder.SetInsertPoint(merge_block);

    return else_value;
}

auto code_generator::if_else_expr(const visitor& visitor, const if_else_expr_node& node) -> llvm::Value* {
    std::cout << "if_else_expr" << std::endl;
    if(node.payload().has_let && any_tree::visit_node(visitor, node.child_at(0)) == nullptr) {
	return nullptr;
    }

    llvm::Value* cond = any_tree::visit_node(visitor, node.child_at(1));
    if(cond == nullptr) {
	return nullptr;
    }

    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(*_context, "then", _scope.function());
    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(*_context, "else", _scope.function());
    llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(*_context, "if_merge", _scope.function());

    _builder.CreateCondBr(cond, then_block, else_block);

    // then
    _builder.SetInsertPoint(then_block);

    llvm::Value* then_value = any_tree::visit_node(visitor, node.child_at(2));
    if(then_value == nullptr) {
	return nullptr;
    }

    _builder.CreateBr(merge_block);
    then_block = _builder.GetInsertBlock();

    // else
    _builder.SetInsertPoint(else_block);

    llvm::Value* else_value = any_tree::visit_node(visitor, node.child_at(3));
    if(else_value == nullptr) {
	return nullptr;
    }

    _builder.CreateBr(merge_block);
    else_block = _builder.GetInsertBlock();

    // merge
    _builder.SetInsertPoint(merge_block);

    llvm::PHINode* phi = _builder.CreatePHI(then_value->getType(), 2, "tmpif");
    phi->addIncoming(then_value, then_block);
    phi->addIncoming(else_value, else_block);

    return phi;
}

auto code_generator::loop_stmt(const visitor& visitor, const loop_node& node) -> llvm::Value* {
    std::cout << "loop" << std::endl;
    if(const auto& let = node.child_at(0); let.has_value() && any_tree::visit_node(visitor, let) == nullptr) {
	return nullptr;
    }

    llvm::BasicBlock* loop = llvm::BasicBlock::Create(*_context, "loop", _scope.function());
    llvm::BasicBlock* after = llvm::BasicBlock::Create(*_context, "loop_after", _scope.function());

    const auto& condition = node.child_at(1);

    if(condition.has_value()) {
	llvm::Value* cond = any_tree::visit_node(visitor, condition);
	if(cond == nullptr) {
	    return nullptr;
	}
	_builder.CreateCondBr(cond, loop, after);
    } else {
	_builder.CreateBr(loop);
    }

    _builder.SetInsertPoint(loop);

    llvm::Value* loop_value = any_tree::visit_node(visitor, node.child_at(3));
    if(loop_value == nullptr) {
	return nullptr;
    }

    if(const auto& post = node.child_at(2); post.has_value() && any_tree::visit_node(visitor, post) == nullptr) {
	return nullptr;
    }

    if(condition.has_value()) {
	llvm::Value* cond = any_tree::visit_node(visitor, condition);
	_builder.CreateCondBr(cond, loop, after);
    } else {
	_builder.CreateBr(loop);
    }

    _builder.SetInsertPoint(after);

    return loop_value;
}

auto code_generator::block(const visitor& visitor, const block_node& node) -> llvm::Value* {
    std::vector<llvm::Value*> statements{};
    statements.reserve(node.children_size());

    std::ranges::transform(
	    node.children(), 
	    std::back_inserter(statements),
	    [&visitor] (const std::any& node) { return any_tree::visit_node(visitor, node); }
    );

    bool invalid_statements = std::ranges::any_of(
	    statements, 
	    [] (llvm::Value* value) { return value == nullptr; }
    );
    if(invalid_statements) {
	std::cout << "invalid statement" << std::endl;
	return nullptr;
    }

    return statements.back();
}

auto code_generator::call(const visitor& visitor, const call_node& node) -> llvm::Value* {
    std::cout << "call" << std::endl;
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
	std::cout << "invalid call argument" << std::endl;
	return nullptr;
    }

    return _builder.CreateCall(callee, param_values, "call");
}

auto code_generator::implicit_cast(const visitor& visitor, const implicit_cast_node& node) -> llvm::Value* {
    std::cout << "cast" << std::endl;
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
    std::cout << "identifier" << std::endl;

    llvm::AllocaInst* inst = _scope.get(node.payload()).value_or(nullptr);

    if(inst == nullptr) {
	return nullptr;
    }

    return _builder.CreateLoad(inst->getAllocatedType(), inst, node.payload());
}

auto code_generator::integer_literal(const integer_literal_node& node) -> llvm::Value* {
    std::cout << "integer_literal" << std::endl;
    return llvm::ConstantInt::get(*_types->get(node.payload().type).value_or(type::type{}), node.payload().value);
}

auto code_generator::floating_literal(const floating_literal_node& node) -> llvm::Value* {
    std::cout << "floating_literal" << std::endl;
    return llvm::ConstantFP::get(*_types->get(node.payload().type).value_or(type::type{}), node.payload().value);
}

auto code_generator::char_literal(const char_literal_node& node) -> llvm::Value* {
    std::cout << "char_literal" << std::endl;
    return llvm::ConstantInt::get(*_types->get(node.payload().type).value_or(type::type{}), node.payload().value);
}

auto code_generator::string_literal(const string_literal_node& node) -> llvm::Value* {
    // not implemented yet
    return nullptr;
}

auto code_generator::bool_literal(const bool_literal_node& node) -> llvm::Value* {
    std::cout << "bool_literal" << std::endl;
    return llvm::ConstantInt::get(*_types->get(node.payload().type).value_or(type::type{}), static_cast<uint64_t>(node.payload().value));
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
	any_tree::make_const_child_visitor<return_statement_node>([this] (const return_statement_node& node)  { return return_statement(_visitor, node); }),
	any_tree::make_const_child_visitor<let_statement_node>   ([this] (const let_statement_node& node)     { return let_statement(_visitor, node); }),
	any_tree::make_const_child_visitor<var_def_node>         ([this] (const var_def_node& node)           { return var_def(_visitor, node); }),
	any_tree::make_const_child_visitor<binary_expr_node>     ([this] (const binary_expr_node& node)       { return binary_expr(_visitor, node); }),
	any_tree::make_const_child_visitor<if_node>              ([this] (const if_node& node)                { return if_stmt(_visitor, node); }),
	any_tree::make_const_child_visitor<if_else_node>         ([this] (const if_else_node& node)           { return if_else_stmt(_visitor, node); }),
	any_tree::make_const_child_visitor<if_else_expr_node>    ([this] (const if_else_expr_node& node)      { return if_else_expr(_visitor, node); }),
	any_tree::make_const_child_visitor<loop_node>            ([this] (const loop_node& node)              { return loop_stmt(_visitor, node); }),
	any_tree::make_const_child_visitor<block_node>           ([this] (const block_node& node)             { return block(_visitor, node); }),
	any_tree::make_const_child_visitor<call_node>            ([this] (const call_node& node)              { return call(_visitor, node); }),
	any_tree::make_const_child_visitor<implicit_cast_node>   ([this] (const implicit_cast_node& node)     { return implicit_cast(_visitor, node); }),
	any_tree::make_const_child_visitor<identifier_node>      ([this] (const identifier_node& node)        { return identifier(node); }),
	any_tree::make_const_child_visitor<integer_literal_node> ([this] (const integer_literal_node& node)   { return integer_literal(node); }),
	any_tree::make_const_child_visitor<floating_literal_node>([this] (const floating_literal_node& node)  { return floating_literal(node); }),
	any_tree::make_const_child_visitor<char_literal_node>    ([this] (const char_literal_node& node)      { return char_literal(node); }),
	any_tree::make_const_child_visitor<string_literal_node>  ([this] (const string_literal_node& node)    { return string_literal(node); }),
	any_tree::make_const_child_visitor<bool_literal_node>    ([this] (const bool_literal_node& node)      { return bool_literal(node); }),
	any_tree::make_const_child_visitor<void>                 ([] () { return nullptr; }),
    };
}
