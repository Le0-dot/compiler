#include <ranges>
#include <algorithm>

#include <any_tree.hpp>

#include "semantic_analyzer.hpp"
#include "tree.hpp"
#include "type/type_id.hpp"


auto semantic_analyzer::file(const visitor& visitor, file_node& node) -> type::type_id {
    bool result = std::ranges::all_of(
	    node.children(), 
	    type::valid, 
	    std::bind_front(any_tree::visit_node<type::type_id>, visitor)
    );
    return result ? type::type_id::good_file : type::type_id::undetermined;
}

auto semantic_analyzer::function(const visitor& visitor, function_node& node) -> type::type_id {
    // get function type
    type::type_id func_type = _types->id(node.payload().params_type, node.payload().return_type);

    // push function and parameters to scope
    _scope.add(node.payload().name, func_type);
    _scope.push();

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

	stmt_node.child_at(0) = insert_impicit_cast(std::move(stmt_node.child_at(0)), node.payload().return_type);
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

    if(auto oper = binary_op.get(lhs_type, rhs_type); type::valid(oper.first)) {
	return oper.first;
    }

    for(const auto& [operands, func]: binary_op) {
	bool should_cast_lhs = operands.first != lhs_type;
	bool should_cast_rhs = operands.second != rhs_type;

	bool can_cast_lhs = _special->cast(lhs_type).get(operands.first).has_value();
	bool can_cast_rhs = _special->cast(rhs_type).get(operands.second).has_value();

	if((should_cast_lhs && !can_cast_lhs) ||
	   (should_cast_rhs && !can_cast_rhs)) {
	    continue;
	}

	if(should_cast_lhs && can_cast_lhs) {
	    insert_impicit_cast(std::move(node.child_at(0)), operands.first);
	}

	if(should_cast_rhs && can_cast_rhs) {
	    insert_impicit_cast(std::move(node.child_at(1)), operands.second);
	}
	
	return func.first;
    }

    return type::type_id::undetermined;
}

auto semantic_analyzer::call(const visitor& visitor, call_node& node) -> type::type_id {
    type::type_id func_type_id = _scope.get(node.payload().callee).value_or(type::type_id::undetermined);
    if(!_types->is_function(func_type_id)) {
	return type::type_id::undetermined;
    }
    
    const type::function_type& func_type = _types->get_function(func_type_id);

    if(node.children().size() != func_type.params().size()) {
	return type::type_id::undetermined;
    }

    auto expr_iter = node.children().begin();
    auto param_iter = func_type.params().begin();
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

	*expr_iter = insert_impicit_cast(std::move(*expr_iter), *param_iter);
    }

    return node.payload().type = func_type.return_type();
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
