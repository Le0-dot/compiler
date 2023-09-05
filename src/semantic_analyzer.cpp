#include <algorithm>
#include <any>
#include <cassert>
#include <iostream>
#include <ranges>

#include <any_tree.hpp>
#include <typeindex>

#include "any_tree/visitor.hpp"
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
    type::type_id func_type = _types.id(node.payload().params_type, node.payload().return_type);

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
	auto& stmt_node = std::any_cast<statement_node&>(stmt);
	bool invalid_return = stmt_node.payload().is_return && tid != node.payload().return_type;

	if(!type::valid(tid) || invalid_return) {
	    // try casting
	    return type::type_id::undetermined;
	    // or
	    // func_type = type::type_id::undetermined
	}
    }

    return func_type;
}

auto semantic_analyzer::statement(const visitor& visitor, statement_node& node) -> type::type_id {
    return any_tree::visit_node(visitor, node.child_at(0));
}

auto semantic_analyzer::binary_expr(const visitor& visitor, binary_expr_node& node) -> type::type_id {
    auto binary_op = _special.binary(node.payload().oper);

    type::type_id lhs_type = any_tree::visit_node(visitor, node.child_at(0));
    type::type_id rhs_type = any_tree::visit_node(visitor, node.child_at(1));

    if(auto oper = binary_op.get(lhs_type, rhs_type); type::valid(oper.first)) {
	return oper.first;
    }

    for(const auto& [in, out]: binary_op) {
	bool can_cast_lhs = _special.cast(lhs_type).get(in.first).has_value();
	bool can_cast_rhs = _special.cast(rhs_type).get(in.second).has_value();
	
	if(can_cast_lhs && can_cast_rhs) {
	    // insert casts
	    return out.first;
	}
    }

    return type::type_id::undetermined;
}

auto semantic_analyzer::call(const visitor& visitor, call_node& node) -> type::type_id {
    type::type_id func_type = _scope.get(node.payload().callee);
    if(!_types.is_function(func_type)) {
	return type::type_id::undetermined;
    }
    
    bool result = std::ranges::all_of(
	    node.children(),
	    type::valid,
	    std::bind_front(any_tree::visit_node<type::type_id>, visitor)
    );

    if(result) {
	return node.payload().type = _types.get_function(func_type).return_type();
    }

    return node.payload().type;
}

auto semantic_analyzer::identifier(identifier_node& node) -> type::type_id {
    return _scope.get(node.payload());
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

semantic_analyzer::semantic_analyzer(special_functions& special, type::registry& types)
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
