#include <iostream>
#include <ranges>
#include <algorithm>
#include <tuple>

#include <any_tree.hpp>

#include "functions.hpp"
#include "semantic_analyzer.hpp"
#include "tree.hpp"
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
    scope_pusher pusher{&_scope};

    auto param_name = node.payload().params.begin();
    auto param_type = node.payload().params_type.begin();
    while(param_name != node.payload().params.end()) {
	_scope.add(*param_name++, *param_type++);
    }

    // visit statements and analyze them
    for(auto& stmt: node.children()) {
	type::type_id tid = any_tree::visit_node(visitor, stmt);

	if(!type::valid(tid)) {
	    return type::type_id::undetermined;
	}

	auto& stmt_node = std::any_cast<statement_node&>(stmt);
	bool invalid_return = stmt_node.payload().is_return && tid != node.payload().return_type;

	if(!invalid_return) {
	    continue;
	}

	auto cast = _special->cast(tid).get(node.payload().return_type);

	if(!cast.has_value()) {
	    return type::type_id::undetermined;
	    // or
	    // func_type = type::type_id::undetermined
	}

	stmt_node.child_at(0) = insert_implicit_cast(std::move(stmt_node.child_at(0)), tid, node.payload().return_type);
    }

    return func_type;
}

auto semantic_analyzer::statement(const visitor& visitor, statement_node& node) -> type::type_id {
    return any_tree::visit_node(visitor, node.child_at(0));
}

auto semantic_analyzer::binary_expr(const visitor& visitor, binary_expr_node& node) -> type::type_id {
    auto binary_op = _special->binary(node.payload().oper);

    type::type_id lhs_type = any_tree::visit_node(visitor, node.child_at(0));
    type::type_id rhs_type = any_tree::visit_node(visitor, node.child_at(1));

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

	// allow only lhs cast or rhs cast not both
	if(type_cast_lhs && type_cast_rhs) {
	    continue;
	}

	bool can_cast_lhs = _special->cast(lhs_type).get(operands.first).has_value();
	bool can_cast_rhs = _special->cast(rhs_type).get(operands.second).has_value();

	// check if operand should be casted but unable to
	if((should_cast_lhs && !can_cast_lhs) ||
	   (should_cast_rhs && !can_cast_rhs)) {
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
    auto [lhs, rhs, expr] = candidates.front();
    node.child_at(0) = insert_implicit_cast(std::move(node.child_at(0)), lhs_type, lhs);
    node.child_at(1) = insert_implicit_cast(std::move(node.child_at(1)), rhs_type, rhs);
    node.payload().lhs = lhs;
    node.payload().rhs = rhs;
    return expr;
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
}

auto semantic_analyzer::bool_literal(bool_literal_node& node) -> type::type_id {
    return node.payload().type = type::type_id::bool_;
}

semantic_analyzer::semantic_analyzer(special_functions* special, type::registry* types)
    : _special{special}
    , _types{types}
{
    _visitor = {
	any_tree::make_child_visitor<file_node>            ([this] (file_node& node)        { return file(_visitor, node); }),
	any_tree::make_child_visitor<function_node>        ([this] (function_node& node)    { return function(_visitor, node); }),
	any_tree::make_child_visitor<statement_node>       ([this] (statement_node& node)   { return statement(_visitor, node); }),
	any_tree::make_child_visitor<binary_expr_node>     ([this] (binary_expr_node& node) { return binary_expr(_visitor, node); }),
	any_tree::make_child_visitor<call_node>            ([this] (call_node& node)        { return call(_visitor, node); }),
	any_tree::make_child_visitor<identifier_node>      ([this] (identifier_node& node)  { return identifier(node); }),
	any_tree::make_child_visitor<integer_literal_node> (integer_literal ),
	any_tree::make_child_visitor<floating_literal_node>(floating_literal),
	any_tree::make_child_visitor<char_literal_node>    (char_literal    ),
	any_tree::make_child_visitor<string_literal_node>  (string_literal  ),
	any_tree::make_child_visitor<bool_literal_node>    (bool_literal    ),
    };
}
