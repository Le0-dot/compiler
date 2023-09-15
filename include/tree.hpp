#pragma once 

#include <any_tree.hpp>
#include <functional>
#include <nlohmann/json.hpp>
#include <ratio>

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

struct var_def_info {
    std::string name;
    type::type_id type;
};

struct binary_expr_info {
    std::string oper;
    type::type_id lhs;
    type::type_id rhs;
};

struct call_info {
    std::string callee;
    type::type_id type;
};

struct if_info {
    bool has_let;
};

template<typename T>
struct literal {
    T value;
    type::type_id type;
};

struct cast_info {
    type::type_id from_type;
    type::type_id to_type;
};

using file_node             = any_tree::dynamic_node<void>;
using let_statement_node    = any_tree::dynamic_node<void, 1>;
using if_block_node	    = any_tree::dynamic_node<void, 2>;
using function_node         = any_tree::dynamic_node<function_info>;
using var_def_node          = any_tree::dynamic_node<var_def_info>;
using call_node             = any_tree::dynamic_node<call_info>;
using return_statement_node = any_tree::static_node<void, 1>;
using binary_expr_node      = any_tree::static_node<binary_expr_info, 2>;
// if's
using if_node               = any_tree::static_node<if_info, 3, 0>;
using if_else_node          = any_tree::static_node<if_info, 4, 1>;
using if_else_expr_node     = any_tree::static_node<if_info, 4, 2>;

using identifier_node       = any_tree::leaf<std::string>;
// literals
using integer_literal_node  = any_tree::leaf<literal<std::uint64_t>>;
using floating_literal_node = any_tree::leaf<literal<double>>;
using char_literal_node     = any_tree::leaf<literal<char>>;
using string_literal_node   = any_tree::leaf<literal<std::string>>;
using bool_literal_node     = any_tree::leaf<literal<bool>>;

using implicit_cast_node = any_tree::static_node<cast_info, 1>;

using json = nlohmann::json;

class tree_builder {
    special_functions* _special;
    type::registry* _types;

    auto file(const json& object)        -> file_node;
    auto function(const json& object)    -> function_node;
    auto stmt(const json& object)        -> std::any;
    auto return_stmt(const json& object) -> return_statement_node;
    auto let_stmt(const json& object)    -> let_statement_node;
    auto var_def(const json& object)     -> var_def_node;
    auto expr(const json& object)        -> std::any;
    auto primary(const json& object)     -> std::any;
    auto if_stmt(const json& object)     -> std::any;
    auto if_expr(const json& object)     -> if_else_expr_node;
    auto if_block(const json& object)    -> if_block_node;
    auto call(const json& object)        -> call_node;

    static auto literal(const json& object)  -> std::any;

    inline auto function_hander()    { return std::bind_front(&tree_builder::function, this); }
    inline auto stmt_hander()        { return std::bind_front(&tree_builder::stmt, this); }
    inline auto return_stmt_hander() { return std::bind_front(&tree_builder::return_stmt, this); }
    inline auto let_stmt_hander()    { return std::bind_front(&tree_builder::let_stmt, this); }
    inline auto var_def_hander()     { return std::bind_front(&tree_builder::var_def, this); }
    inline auto expr_hander()        { return std::bind_front(&tree_builder::expr, this); }
    inline auto if_handler()         { return std::bind_front(&tree_builder::if_stmt, this); }
    inline auto if_expr_handler()    { return std::bind_front(&tree_builder::if_expr, this); }
    inline auto call_hander()        { return std::bind_front(&tree_builder::call, this); }

    auto operator_resolution(std::span<std::any> primaries, std::span<std::string> ops) -> std::any;

public:
    tree_builder(special_functions* special, type::registry* types)
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

auto insert_implicit_cast(std::any&& node, type::type_id from_type, type::type_id to_type) -> std::any;
