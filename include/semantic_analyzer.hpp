#pragma once

#include <any_tree.hpp>

#include "any_tree/visitor.hpp"
#include "functions.hpp"
#include "tree.hpp"
#include "scope.hpp"
#include "type/type_id.hpp"
#include "type/registry.hpp"


class semantic_analyzer {
public:
    using visitor = any_tree::children_visitor<type::type_id>;

private:
    scope_manager<type::type_id> _scope{};
    special_functions* _special;
    type::registry* _types;

    visitor _visitor;

    auto file       (const visitor& visitor, file_node& node)        -> type::type_id;
    auto function   (const visitor& visitor, function_node& node)    -> type::type_id;
    auto statement  (const visitor& visitor, statement_node& node)   -> type::type_id;
    auto binary_expr(const visitor& visitor, binary_expr_node& node) -> type::type_id;
    auto call       (const visitor& visitor, call_node& node)        -> type::type_id;

    auto identifier (identifier_node& node) -> type::type_id;

    static auto integer_literal (integer_literal_node& node)  -> type::type_id;
    static auto floating_literal(floating_literal_node& node) -> type::type_id;
    static auto char_literal    (char_literal_node& node)     -> type::type_id;
    static auto string_literal  (string_literal_node& node)   -> type::type_id;
    static auto bool_literal    (bool_literal_node& node)     -> type::type_id;

public:
    semantic_analyzer(special_functions* special, type::registry* types);

    auto get_visitor() -> visitor& { return _visitor; }
};
