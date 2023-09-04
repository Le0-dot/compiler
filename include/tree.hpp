#include <any_tree.hpp>
#include <nlohmann/json.hpp>

#include "type/type_id.hpp"
#include "type/registry.hpp"
#include "functions.hpp"


struct variable {
    std::string name;
    type::type_id tid;
};

struct function_info {
    std::string name;
    std::vector<variable> params;
    type::type_id return_type;
};

struct statement_info {
    bool is_return;
};

struct binary_expr_info {
    std::string oper;
};

struct call_info {
    std::string callee;
};

using file_node        = any_tree::dynamic_node<void>;
using function_node    = any_tree::dynamic_node<function_info>;
using statement_node   = any_tree::static_node<statement_info, 1>;
using binary_expr_node = any_tree::static_node<binary_expr_info, 2>;
using call_node        = any_tree::dynamic_node<call_info>;
using identifier_node  = any_tree::leaf<std::string>;
// literals
using integer_literal_node  = any_tree::leaf<std::uint64_t>;
using floating_literal_node = any_tree::leaf<double>;
using char_literal_node     = any_tree::leaf<char>;
using string_literal_node   = any_tree::leaf<std::string>;
using bool_literal_node     = any_tree::leaf<bool>;

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
    auto var_def(const json& object)  -> variable;

    static auto literal(const json& object)  -> std::any;

    inline auto function_hander() { return std::bind_front(&tree_builder::function, this); }
    inline auto stmt_hander()     { return std::bind_front(&tree_builder::stmt, this); }
    inline auto expr_hander()     { return std::bind_front(&tree_builder::expr, this); }
    inline auto call_hander()     { return std::bind_front(&tree_builder::call, this); }
    inline auto var_def_hander()  { return std::bind_front(&tree_builder::var_def, this); }

    auto operator_resolution(std::span<std::any> primaries, std::span<std::string> ops) -> std::any;

public:
    tree_builder(special_functions& special, type::registry& types)
	: _special{special}
	, _types{types}
    {}

    inline auto operator()(const json& object) -> std::any { return file(object); }
};
