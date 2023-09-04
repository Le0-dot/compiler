#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <unordered_map>
#include <functional>
#include <vector>
#include <any>

#include "tree.hpp"

auto tree_builder::file(const json& object) -> file_node {
    file_node node{};

    std::ranges::transform(object["functions"], std::back_inserter(node.children()), function_hander());

    return node;
}

auto tree_builder::function(const json& object) -> function_node {
    function_node node{};

    node.payload().name = object["funcName"].template get<std::string>();
    node.payload().return_type = _types.id(object["funcReturn"].template get<std::string>());
    std::ranges::transform(object["funcParams"], std::back_inserter(node.payload().params), var_def_hander());
    std::ranges::transform(object["funcBody"], std::back_inserter(node.children()), stmt_hander());

    return node;
}

auto tree_builder::stmt(const json& object) -> statement_node {
    statement_node node{object["return"].template get<bool>()};
    node.children()[0] = expr(object["expr"]);
    return node;
}

auto tree_builder::operator_resolution(std::span<std::any> primaries, std::span<std::string> ops) -> std::any {
    if(ops.empty()) {
	return primaries.front();
    }

    auto op_iter = std::ranges::min_element(ops, {}, [this] (const std::string& oper) { return _special.binary(oper).precedense(); });
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
    // or
    //std::ranges::transform(object["rhs"] | std::ranges::views::transform([] (auto& object) { return object["op"]; }), std::back_inserter(ops), &json::template get<std::string>);
    //std::ranges::transform(object["rhs"] | std::ranges::views::transform([] (auto& object) { return object["rhsOperand"]; }), std::back_inserter(primaries), primary);

    return operator_resolution(std::span{primaries}, std::span{ops});
}

auto tree_builder::primary(const json& object) -> std::any {
    // should replace with constexpr std::flat_map once c++23 is out
    static std::unordered_map<std::string, std::function<std::any(const json&)>> handlers{
	{"id",      [] (const json& object) { return identifier_node{object.template get<std::string>()}; }},
	{"parens",  expr_hander()},
	{"call",    call_hander()},
	{"literal", literal},
    };

    const std::string& type = object["type"].template get<std::string>();
    return handlers[type](object["val"]);
}

auto tree_builder::call(const json& object) -> call_node {
    call_node node{object["callable"].template get<std::string>()};

    std::ranges::transform(object["callParams"], std::back_inserter(node.children()), expr_hander());

    return node;
}

auto tree_builder::var_def(const json& object) -> variable {
    return {
	.name = object["varName"].template get<std::string>(),
	.tid = _types.id(object["varType"].template get<std::string>()),
    };
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
