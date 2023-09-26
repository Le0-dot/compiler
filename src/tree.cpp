#include <algorithm>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <functional>
#include <vector>
#include <optional>

#include "any_tree/visitor.hpp"
#include "tree.hpp"
#include "type/type_id.hpp"


auto tree_builder::file(const json& object) -> file_node {
    file_node node{};

    std::ranges::transform(object["functions"], std::back_inserter(node.children()), function_hander());

    return node;
}

auto tree_builder::function(const json& object) -> function_node {
    function_node node{};

    node.payload().name = object["funcName"].template get<std::string>();
    node.payload().return_type = _types->id(object["funcReturn"].template get<std::string>());

    node.payload().params.reserve(object["funcParams"].size());
    node.payload().params_type.reserve(object["funcParams"].size());

    std::ranges::transform(
	    object["funcParams"], 
	    std::back_inserter(node.payload().params), 
	    [] (const json& object) { 
		return object["argName"].template get<std::string>(); 
	    }
    );
    std::ranges::transform(
	    object["funcParams"], 
	    std::back_inserter(node.payload().params_type), 
	    [this] (const json& object) { 
		return _types->id(object["argType"].template get<std::string>()); 
	    }
    );

    node.child_at(0) = block(object["funcBody"]);

    return node;
}

auto tree_builder::stmt(const json& object) -> std::any {
    static std::unordered_map<std::string, std::function<std::any(const json&)>> handlers{
	{"IgnoreResultStmt",       expr_hander()},
	{"ReturnStmt",             return_stmt_hander()},
	{"VariableDefinitionStmt", let_stmt_hander()},
	{"IfStmt",                 if_handler()},
	{"LoopStmt",               loop_hander()},
    };

    return handlers[object["tag"]](object["contents"]);
}

auto tree_builder::return_stmt(const json& object) -> return_statement_node {
    return_statement_node node{}; 
    node.child_at(0) = expr(object); 
    return node;
}

auto tree_builder::let_stmt(const json& object) -> let_statement_node {
    let_statement_node node{};

    std::ranges::transform(object, std::back_inserter(node.children()), var_def_hander());

    return node;
}

auto tree_builder::var_def(const json& object) -> var_def_node {
    var_def_node node{};

    node.payload().name = object["varName"].template get<std::string>();

    if(const json& type = object["varType"]; type.is_null()) {
	node.payload().type = type::type_id::unset;
    } else {
	node.payload().type = _types->id(type.template get<std::string>());
    }

    if(const json& value = object["varValue"]; !value.is_null()) {
	node.children().reserve(1);
	node.children().emplace_back(expr(object["varValue"]));
    }
    return node;
}

auto tree_builder::operator_resolution(std::span<std::any> primaries, std::span<std::string> ops) -> std::any {
    if(ops.empty()) {
	return primaries.front();
    }

    auto op_iter = std::ranges::min_element(ops, {}, [this] (const std::string& oper) { return _special->binary(oper).precedense(); });
    auto op_pos = op_iter - ops.begin();

    binary_expr_node node{*op_iter};
    node.children()[0] = operator_resolution(primaries.first(op_pos + 1), ops.first(op_pos));
    node.children()[1] = operator_resolution(primaries.subspan(op_pos + 1), ops.subspan(op_pos + 1));

    return node;
}

auto tree_builder::expr(const json& object) -> std::any {
    auto lhs = primary(object["lhs"]);

    if(object["rhs"].empty()) {
	return lhs;
    }
    
    std::vector<std::any> primaries{};
    std::vector<std::string> ops{};

    primaries.reserve(object["rhs"].size() + 1);
    ops.reserve(object["rhs"].size());

    primaries.emplace_back(std::move(lhs));

    std::ranges::for_each(object["rhs"], [&ops, &primaries, this] (const auto& rhs) {
	ops.emplace_back(rhs["op"].template get<std::string>());
	primaries.emplace_back(primary(rhs["rhsOperand"]));
    });

    return operator_resolution(std::span{primaries}, std::span{ops});
}

auto tree_builder::primary(const json& object) -> std::any {
    // should replace with constexpr std::flat_map once c++23 is out
    static std::unordered_map<std::string, std::function<std::any(const json&)>> handlers{
	{"PrimaryId",      [] (const json& object) { return identifier_node{object.template get<std::string>()}; }},
	{"PrimaryParens",  expr_hander()},
	{"PrimaryCall",    call_hander()},
	{"PrimaryLiteral", literal},
	{"PrimaryIf",      if_expr_handler()},
    };

    const std::string& type = object["tag"].template get<std::string>();
    return handlers[type](object["contents"]);
}

auto tree_builder::if_stmt(const json& object) -> std::any {
    if_info info{!object["ifScopeVar"].is_null()};

    std::any let{};
    if(info.has_let) {
	let = let_stmt(object["ifScopeVar"]);
    }

    std::any cond = expr(object["ifCond"]);
    std::any then = block(object["thenBlock"]);

    if(const json& else_blk = object["elseBlock"]; !else_blk.is_null()) {
	if_else_node node{info};
	node.child_at(0) = let;
	node.child_at(1) = cond;
	node.child_at(2) = then;
	node.child_at(3) = block(else_blk);
	return node;
    } 

    if_node node{info};
    node.child_at(0) = let;
    node.child_at(1) = cond;
    node.child_at(2) = then;
    return node;
}

auto tree_builder::if_expr(const json& object) -> if_else_expr_node {
    if(object["elseBlock"].is_null()) {
	// temporary
	throw int{};
    }

    if_else_expr_node node{!object["ifScopeVar"].is_null()};

    if(node.payload().has_let) {
	node.child_at(0) = let_stmt(object["ifScopeVar"]);
    }

    node.child_at(1) = expr(object["ifCond"]);
    node.child_at(2) = block(object["thenBlock"]);
    node.child_at(3) = block(object["elseBlock"]);
    return node;
}

auto tree_builder::call(const json& object) -> call_node {
    call_node node{object["callable"].template get<std::string>()};

    std::ranges::transform(object["callParams"], std::back_inserter(node.children()), expr_hander());

    return node;
}

auto tree_builder::loop(const json& object) -> loop_node {
    loop_node node{};

    if (const auto& var = object["loopScopeVar"]; !var.is_null()) {
        node.child_at(0) = let_stmt(var);
    }
    
    if(const auto& cond = object["loopCond"]; !cond.is_null()) {
	node.child_at(1) = expr(cond);
    }

    if(const auto& post = object["loopPostIter"]; !post.is_null()) {
	node.child_at(2) = expr(post);
    }

    node.child_at(3) = block(object["loopBody"]);

    return node;
}

auto tree_builder::literal(const json& object) -> std::any {
    // should replace with constexpr std::flat_map once c++23 is out
    static std::unordered_map<std::string, std::function<std::any(const json&)>> handlers{
	{"IntegerLiteral", [] (const json& object) { return integer_literal_node {object.template get<std::uint64_t>()}; }},
	{"FloatLiteral",   [] (const json& object) { return floating_literal_node{object.template get<double>()};        }},
	{"CharLiteral",    [] (const json& object) { return char_literal_node    {object.template get<char>()};          }},
	{"StringLiteral",  [] (const json& object) { return string_literal_node  {object.template get<std::string>()};   }},
	{"BoolLiteral",    [] (const json& object) { return bool_literal_node    {object.template get<bool>()};          }},
    };

    return handlers[object["tag"]](object["contents"]);
}

auto tree_builder::block(const json& object) -> block_node {
    block_node node{};

    std::ranges::transform(object, std::back_inserter(node.children()), stmt_hander());

    return node;
}


auto insert_implicit_cast(std::any &&node, type::type_id from_type, type::type_id to_type) -> std::any {
    if(from_type == to_type) { 
	return node;
    }
    if(type::is_literal(from_type)) {
	any_tree::children_visitor<void> visitor{
	    any_tree::make_child_visitor<integer_literal_node>([to_type] (integer_literal_node& node) { node.payload().type = to_type; }),
	    any_tree::make_child_visitor<floating_literal_node>([to_type] (floating_literal_node& node) { node.payload().type = to_type; }),
	};
	any_tree::visit_node(visitor, node);
	return node;
    }

    implicit_cast_node cast{from_type, to_type};
    cast.child_at(0) = std::move(node);
    return cast;
}
