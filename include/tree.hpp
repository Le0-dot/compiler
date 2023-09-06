#pragma once 

#include <any_tree.hpp>
#include <nlohmann/json.hpp>

#include "any_tree/node.hpp"
#include "type/type_id.hpp"
#include "type/registry.hpp"
#include "functions.hpp"


struct function_info {
    std::string name;
    std::vector<std::string> params;
    std::vector<type::type_id> params_type;
    type::type_id return_type;
};

struct statement_info {
    bool is_return;
};

struct binary_expr_info {
    std::string oper;
    type::type_id type;
};

struct call_info {
    std::string callee;
    type::type_id type;
};

template<typename T>
struct literal {
    T value;
    type::type_id type;
};

using file_node        = any_tree::dynamic_node<void>;
using function_node    = any_tree::dynamic_node<function_info>;
using statement_node   = any_tree::static_node<statement_info, 1>;
using binary_expr_node = any_tree::static_node<binary_expr_info, 2>;
using call_node        = any_tree::dynamic_node<call_info>;
using identifier_node  = any_tree::leaf<std::string>;
// literals
using integer_literal_node  = any_tree::leaf<literal<std::uint64_t>>;
using floating_literal_node = any_tree::leaf<literal<double>>;
using char_literal_node     = any_tree::leaf<literal<char>>;
using string_literal_node   = any_tree::leaf<literal<std::string>>;
using bool_literal_node     = any_tree::leaf<literal<bool>>;

using implicit_cast_node = any_tree::static_node<type::type_id, 1>;

using json = nlohmann::json;

class tree_builder {
    special_functions& _special;
    type::registry& _types;

    auto file(const json& object)     -> file_node;
    auto function(const json& object) -> function_node;
    auto stmt(const json& object)     -> statement_node;
    auto expr(const json& object)     -> std::any;
    auto primary(const json& object)  -> std::any;
    auto call(const json& object)     -> call_node;

    static auto literal(const json& object)  -> std::any;

    inline auto function_hander() { return std::bind_front(&tree_builder::function, this); }
    inline auto stmt_hander()     { return std::bind_front(&tree_builder::stmt, this); }
    inline auto expr_hander()     { return std::bind_front(&tree_builder::expr, this); }
    inline auto call_hander()     { return std::bind_front(&tree_builder::call, this); }

    auto operator_resolution(std::span<std::any> primaries, std::span<std::string> ops) -> std::any;

public:
    tree_builder(special_functions& special, type::registry& types)
	: _special{special}
	, _types{types}
    {}

    tree_builder()                      = delete;
    tree_builder(const tree_builder&)   = delete;
    tree_builder(tree_builder&&)        = delete;
    auto operator=(const tree_builder&) = delete;
    auto operator=(tree_builder&&)      = delete;
    ~tree_builder()                     = default;


    inline auto operator()(const json& object) -> std::any { return file(object); }
};

auto insert_impicit_cast(std::any&& node, type::type_id to_type) -> implicit_cast_node;
