#include <iostream>
#include <ranges>
#include <algorithm>
#include <tuple>

#include <any_tree.hpp>

#include "any_tree/visitor.hpp"
#include "functions.hpp"
#include "semantic_analyzer.hpp"
#include "tree.hpp"
#include "type/type.hpp"
#include "type/type_id.hpp"


auto semantic_analyzer::file(const visitor& visitor, file_node& node) -> type::type_id {
    bool result = std::ranges::all_of(
	    node.children(), 
	    type::valid, 
	    [&visitor] (std::any& node) { return any_tree::visit_node(visitor, node); }
    );
    return result ? type::type_id::good_file : type::type_id::undetermined;
}

auto semantic_analyzer::function(const visitor& visitor, function_node& node) -> type::type_id {
    // get function type
    type::type_id func_type = _types->id(node.payload().params_type, node.payload().return_type);

    // push function and parameters to scope
    _scope.add(node.payload().name, func_type);
    scope_pusher pusher{&_scope, func_type};

    auto param_name = node.payload().params.begin();
    auto param_type = node.payload().params_type.begin();
    while(param_name != node.payload().params.end()) {
	_scope.add(*param_name++, *param_type++);
    }

    // visit statements and analyze them
    auto type = any_tree::visit_node(visitor, node.child_at(0));
    if(!type::valid(type)) {
	return type::type_id::undetermined;
    }

    return func_type;
}

auto semantic_analyzer::return_statement(const visitor& visitor, return_statement_node& node) -> type::type_id {
    type::type_id stmt_type =  any_tree::visit_node(visitor, node.child_at(0));
    type::type_id func_return = _types->get_function(_scope.function())->return_type();
    
    if(stmt_type == func_return) {
	return stmt_type;
    }

    auto cast = _special->cast(stmt_type).get(func_return);
    if(!cast.has_value()) {
	return type::type_id::undetermined;
    }

    node.child_at(0) = insert_implicit_cast(std::move(node.child_at(0)), stmt_type, func_return);
    return type::type_id::good_stmt;
}

auto semantic_analyzer::let_statement(const visitor& visitor, let_statement_node& node) -> type::type_id {
    bool result = std::ranges::all_of(
	    node.children(),
	    type::valid,
	    [&visitor] (std::any& child) { return any_tree::visit_node(visitor, child); }
    );
    return result ? type::type_id::good_stmt : type::type_id::undetermined;
}

auto var_def_with_expr(const semantic_analyzer::visitor& visitor, var_def_node& node, special_functions* special) -> type::type_id {
    type::type_id expr_type = any_tree::visit_node(visitor, node.child_at(0));

    if(!type::valid(expr_type)) {
	return type::type_id::undetermined;
    }

    if(auto default_t = type::default_type(expr_type); type::is_literal(expr_type)) {
	node.child_at(0) = insert_implicit_cast(std::move(node.child_at(0)), expr_type, default_t);
	expr_type = default_t;
    }

    if(node.payload().type == type::type_id::unset) {
	return node.payload().type = expr_type;
    }

    if(node.payload().type == expr_type) {
	return expr_type;
    }

    auto cast = special->cast(expr_type).get(node.payload().type);
    if(!cast.has_value()) {
	return type::type_id::undetermined;
    }

    node.child_at(0) = insert_implicit_cast(std::move(node.child_at(0)), expr_type, node.payload().type);
    return node.payload().type;
}

auto var_def_without_expr(var_def_node& node) -> type::type_id {
    if(!type::valid(node.payload().type)) {
	return type::type_id::undetermined;
    }
    return node.payload().type;
}

auto semantic_analyzer::var_def(const visitor& visitor, var_def_node& node) -> type::type_id {
    type::type_id type{};

    if(node.children().empty()) {
	type = var_def_without_expr(node);
    } else {
	type = var_def_with_expr(visitor, node, _special);
    }

    if(!type::valid(type)) {
	return type::type_id::undetermined;
    }

    _scope.add(node.payload().name, node.payload().type);
    return type::type_id::good_stmt;
}

auto semantic_analyzer::binary_expr(const visitor& visitor, binary_expr_node& node) -> type::type_id {
    type::type_id lhs_type = any_tree::visit_node(visitor, node.child_at(0));
    type::type_id rhs_type = any_tree::visit_node(visitor, node.child_at(1));

    if(node.payload().oper == "=") {
	if(lhs_type == rhs_type) {
	    return lhs_type;
	}

	if(auto cast = _special->cast(rhs_type).get(lhs_type); cast.has_value()) {
	    node.child_at(1) = insert_implicit_cast(std::move(node.child_at(1)), rhs_type, lhs_type);
	    return lhs_type;
	}
	
	return type::type_id::undetermined;
    }

    auto binary_op = _special->binary(node.payload().oper);

    if(binary_op.empty()) {
	return type::type_id::undetermined;
    }

    // try find operator with exact type definition
    if(const auto& [type, _] = binary_op.get(lhs_type, rhs_type); type::valid(type)) {
	node.payload().lhs = lhs_type;
	node.payload().rhs = rhs_type;
	return type;
    }

    std::vector<std::tuple<type::type_id, type::type_id, type::type_id>> candidates{};

    // add possible operators to candidate list
    for(const auto& [operands, func]: binary_op) {
	bool should_cast_lhs = operands.first != lhs_type;
	bool should_cast_rhs = operands.second != rhs_type;

	// cast is from one type to another, not from literal to specific type
	bool type_cast_lhs = should_cast_lhs && !type::is_literal(lhs_type);
	bool type_cast_rhs = should_cast_rhs && !type::is_literal(rhs_type);
	bool must_cast_both = type_cast_lhs && type_cast_rhs;

	// allow only lhs cast or rhs cast not both
	if(must_cast_both) {
	    continue;
	}

	bool can_cast_lhs = _special->cast(lhs_type).get(operands.first).has_value();
	bool can_cast_rhs = _special->cast(rhs_type).get(operands.second).has_value();

	bool impossible_cast = (should_cast_lhs && !can_cast_lhs) || (should_cast_rhs && !can_cast_rhs);

	// check if operand should be casted but unable to
	if(impossible_cast) {
	    continue;
	}

	candidates.emplace_back(operands.first, operands.second, func.return_type);
    }

    if(candidates.empty()) {
	return type::type_id::undetermined;
    }

    // bring candidates with lesser literal casts to the beginning
    // i.e lhs -> u8, lhs -> u16, ..., lhs -> fp64
    if(type::is_literal(lhs_type)) {
	std::ranges::sort(candidates, {}, [] (auto candidate) { return std::get<0>(candidate); });
    }
    // same with rhs literal
    if(type::is_literal(rhs_type)) {
	std::ranges::stable_sort(candidates, {}, [] (auto candidate) { return std::get<1>(candidate); });
    }

    // best candidate will be the first element
    auto& [lhs, rhs, expr] = candidates.front();
    node.child_at(0) = insert_implicit_cast(std::move(node.child_at(0)), lhs_type, lhs);
    node.child_at(1) = insert_implicit_cast(std::move(node.child_at(1)), rhs_type, rhs);
    node.payload().lhs = lhs;
    node.payload().rhs = rhs;
    return expr;
}

auto semantic_analyzer::if_stmt(const visitor& visitor, if_node& node) -> type::type_id {
    scope_pusher pusher{&_scope};

    auto let_t = type::type_id::unset;
    if(node.payload().has_let) {
	let_t = any_tree::visit_node(visitor, node.child_at(0));
    }

    auto cond_t = any_tree::visit_node(visitor, node.child_at(1));
    auto then_t = any_tree::visit_node(visitor, node.child_at(2));

    bool valid = type::valid(let_t) && type::valid(cond_t) && type::valid(then_t);
    if(!valid) {
	return type::type_id::undetermined;
    }

    if(cond_t != type::type_id::bool_) {
	auto cast = _special->cast(cond_t).get(type::type_id::bool_);
	if(!cast.has_value()) {
	    return type::type_id::undetermined;
	}

	node.child_at(1) = insert_implicit_cast(std::move(node.child_at(1)), cond_t, type::type_id::bool_);
    }

    return type::type_id::good_stmt;
}

auto semantic_analyzer::if_else_stmt(const visitor& visitor, if_else_node& node) -> type::type_id {
    scope_pusher pusher{&_scope};

    auto let_t = type::type_id::unset;
    if(node.payload().has_let) {
	let_t = any_tree::visit_node(visitor, node.child_at(0));
    }

    auto cond_t = any_tree::visit_node(visitor, node.child_at(1));
    auto then_t = any_tree::visit_node(visitor, node.child_at(2));
    auto else_t = any_tree::visit_node(visitor, node.child_at(3));

    bool valid = type::valid(let_t) && type::valid(cond_t) && type::valid(then_t) && type::valid(else_t);
    if(!valid) {
	return type::type_id::undetermined;
    }

    if(cond_t != type::type_id::bool_) {
	auto cast = _special->cast(cond_t).get(type::type_id::bool_);
	if(!cast.has_value()) {
	    return type::type_id::undetermined;
	}

	node.child_at(1) = insert_implicit_cast(std::move(node.child_at(1)), cond_t, type::type_id::bool_);
    }

    return type::type_id::good_stmt;
}

auto semantic_analyzer::if_else_expr(const visitor& visitor, if_else_expr_node& node) -> type::type_id {
    scope_pusher pusher{&_scope};

    auto let_t = type::type_id::unset;
    if(node.payload().has_let) {
	let_t = any_tree::visit_node(visitor, node.child_at(0));
    }

    auto cond_t = any_tree::visit_node(visitor, node.child_at(1));
    auto then_t = any_tree::visit_node(visitor, node.child_at(2));
    auto else_t = any_tree::visit_node(visitor, node.child_at(3));

    bool valid = type::valid(let_t) && type::valid(cond_t) && type::valid(then_t) && type::valid(else_t);
    if(!valid) {
	return type::type_id::undetermined;
    }

    if(cond_t != type::type_id::bool_) {
	auto cast = _special->cast(cond_t).get(type::type_id::bool_);
	if(!cast.has_value()) {
	    return type::type_id::undetermined;
	}

	node.child_at(1) = insert_implicit_cast(std::move(node.child_at(1)), cond_t, type::type_id::bool_);
    }

    if(then_t == else_t) {
	return then_t;
    }

    type::type_id common{};
    auto& then_blk = std::any_cast<block_node&>(node.child_at(2));
    auto& else_blk = std::any_cast<block_node&>(node.child_at(3));

    if(type::is_literal(then_t) && !type::is_literal(else_t)) {
	then_blk.children().back() = insert_implicit_cast(std::move(then_blk.children().back()), then_t, else_t);
	common = else_t;
    }

    if(!type::is_literal(then_t) && type::is_literal(else_t)) {
	else_blk.children().back() = insert_implicit_cast(std::move(else_blk.children().back()), else_t, then_t);
	common = then_t;
    }

    if(type::is_literal(then_t) && type::is_literal(else_t)) {
	// TODO: find common type of 2 literals
	return type::type_id::undetermined;
    }

    if(!type::valid(common)) {
	return type::type_id::undetermined;
    }

    return common;
}

auto semantic_analyzer::loop_stmt(const visitor& visitor, loop_node& node) -> type::type_id {
    scope_pusher pusher(&_scope);

    auto let_t = type::type_id::unset;
    if(node.child_at(0).has_value()) {
	let_t = any_tree::visit_node(visitor, node.child_at(0));
    }

    auto cond_t = type::type_id::unset;
    if(node.child_at(1).has_value()) {
	cond_t = any_tree::visit_node(visitor, node.child_at(1));
    }

    auto post_t = type::type_id::unset;
    if(node.child_at(2).has_value()) {
	post_t = any_tree::visit_node(visitor, node.child_at(2));
    }

    auto body_t = any_tree::visit_node(visitor, node.child_at(3));

    bool valid = type::valid(let_t) && type::valid(cond_t) && type::valid(post_t) && type::valid(body_t);
    if(!valid) {
	return type::type_id::undetermined;
    }

    if(cond_t != type::type_id::bool_) {
	auto cast = _special->cast(cond_t).get(type::type_id::bool_);
	if(!cast.has_value()) {
	    return type::type_id::undetermined;
	}

	node.child_at(1) = insert_implicit_cast(std::move(node.child_at(1)), cond_t, type::type_id::bool_);
    }

    return type::type_id::good_stmt;
}

auto semantic_analyzer::block(const visitor& visitor, block_node& node) -> type::type_id {
    auto children = node.children() | std::ranges::views::transform(
	    [&visitor] (std::any& child) { return any_tree::visit_node(visitor, child); }
    );

    return std::accumulate(children.begin(), children.end(), type::type_id::unset,
	    [] (type::type_id left, type::type_id right) {
		if(!type::valid(left) || !type::valid(right)) {
		    return type::type_id::undetermined;
		}

		return right;
	    }
    );
}

auto semantic_analyzer::call(const visitor& visitor, call_node& node) -> type::type_id {
    type::type_id func_type_id = _scope.get(node.payload().callee).value_or(type::type_id::undetermined);
    const type::function_type* func_type = _types->get_function(func_type_id);
    if(func_type == nullptr) {
	return type::type_id::undetermined;
    }

    if(node.children().size() != func_type->params().size()) {
	return type::type_id::undetermined;
    }

    auto expr_iter = node.children().begin();
    auto param_iter = func_type->params().begin();
    for(; expr_iter != node.children().end(); ++expr_iter, ++param_iter) {
	type::type_id expr_type = any_tree::visit_node(visitor, *expr_iter);

	if(!type::valid(expr_type)) {
	    return type::type_id::undetermined;
	}

	if(expr_type == *param_iter) {
	    continue;
	}

	auto cast = _special->cast(expr_type).get(*param_iter);

	if(!cast.has_value()) {
	    return type::type_id::undetermined;
	}

	*expr_iter = insert_implicit_cast(std::move(*expr_iter), expr_type, *param_iter);
    }

    return node.payload().type = func_type->return_type();
}

auto semantic_analyzer::identifier(identifier_node& node) -> type::type_id {
    return _scope.get(node.payload()).value_or(type::type_id::undetermined);
}

auto semantic_analyzer::integer_literal(integer_literal_node& node) -> type::type_id {
    return node.payload().type = type::type_id::u_literal;
}

auto semantic_analyzer::floating_literal(floating_literal_node& node) -> type::type_id {
    return node.payload().type = type::type_id::fp_literal;
}

auto semantic_analyzer::char_literal(char_literal_node& node) -> type::type_id {
    return node.payload().type = type::type_id::char_;
}

auto semantic_analyzer::string_literal(string_literal_node& node) -> type::type_id {
    // not impolemented yet
    return type::type_id::undetermined;
}

auto semantic_analyzer::bool_literal(bool_literal_node& node) -> type::type_id {
    return node.payload().type = type::type_id::bool_;
}

semantic_analyzer::semantic_analyzer(special_functions* special, type::registry* types)
    : _special{special}
    , _types{types}
{
    _visitor = {
	any_tree::make_child_visitor<file_node>            ([this] (file_node& node)             { return file(_visitor, node); }),
	any_tree::make_child_visitor<function_node>        ([this] (function_node& node)         { return function(_visitor, node); }),
	any_tree::make_child_visitor<return_statement_node>([this] (return_statement_node& node) { return return_statement(_visitor, node); }),
	any_tree::make_child_visitor<let_statement_node>   ([this] (let_statement_node& node)    { return let_statement(_visitor, node); }),
	any_tree::make_child_visitor<var_def_node>         ([this] (var_def_node& node)          { return var_def(_visitor, node); }),
	any_tree::make_child_visitor<binary_expr_node>     ([this] (binary_expr_node& node)      { return binary_expr(_visitor, node); }),
	any_tree::make_child_visitor<if_node>              ([this] (if_node& node)               { return if_stmt(_visitor, node); }),
	any_tree::make_child_visitor<if_else_node>         ([this] (if_else_node& node)          { return if_else_stmt(_visitor, node); }),
	any_tree::make_child_visitor<if_else_expr_node>    ([this] (if_else_expr_node& node)     { return if_else_expr(_visitor, node); }),
	any_tree::make_child_visitor<loop_node>            ([this] (loop_node& node)             { return loop_stmt(_visitor, node); }),
	any_tree::make_child_visitor<block_node>           ([this] (block_node& node)            { return block(_visitor, node); }),
	any_tree::make_child_visitor<call_node>            ([this] (call_node& node)             { return call(_visitor, node); }),
	any_tree::make_child_visitor<identifier_node>      ([this] (identifier_node& node)       { return identifier(node); }),
	any_tree::make_child_visitor<integer_literal_node> (integer_literal ),
	any_tree::make_child_visitor<floating_literal_node>(floating_literal),
	any_tree::make_child_visitor<char_literal_node>    (char_literal    ),
	any_tree::make_child_visitor<string_literal_node>  (string_literal  ),
	any_tree::make_child_visitor<bool_literal_node>    (bool_literal    ),
	any_tree::make_child_visitor<void>                 ([] () { return type::type_id::undetermined; }),
    };
}
