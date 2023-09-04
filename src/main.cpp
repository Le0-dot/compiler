#include <iostream>
#include <fstream>

#include <llvm/IR/LLVMContext.h>
#include <nlohmann/json.hpp>
#include <any_tree.hpp>
#include <type_traits>

#include "tree.hpp"
#include "type/registry.hpp"
#include "functions.hpp"
#include "type/type_id.hpp"

void tabs(std::size_t n) {
    for(auto i = 0U; i < n; ++i) {
	std::cout << '\t';
    }
}

auto operator<<(std::ostream& out, type::type_id tid) -> std::ostream& {
    return out << static_cast<std::underlying_type_t<type::type_id>>(tid);
}

auto main(int argc, char** argv) -> int {
    if(argc != 2) {
	return -1;
    }

    std::ifstream file{argv[1]};
    json json = json::parse(file);

    special_functions functions{};

    functions.new_binary("+", 0);
    functions.new_binary("-", 0);
    functions.new_binary("*", 1);
    functions.new_binary("/", 1);

    llvm::LLVMContext context{};
    type::registry types{context};

    types.make_alias("",     type::type_id::void_);
    types.make_alias("bool", type::type_id::bool_);
    types.make_alias("char", type::type_id::char_);
    types.make_alias("u8",   type::type_id::u8);
    types.make_alias("u16",  type::type_id::u16);
    types.make_alias("u32",  type::type_id::u32);
    types.make_alias("u64",  type::type_id::u64);
    types.make_alias("i8",   type::type_id::i8);
    types.make_alias("i16",  type::type_id::i16);
    types.make_alias("i32",  type::type_id::i32);
    types.make_alias("i64",  type::type_id::i64);
    types.make_alias("f32",  type::type_id::fp32);
    types.make_alias("f64",  type::type_id::fp64);

    auto tree = tree_builder{functions, types}(json);


    std::size_t tab{};
    any_tree::children_visitor<void> visitor {
	any_tree::make_child_visitor<file_node>([&visitor, &tab] (file_node& n) { 
		std::cout << "file" << std::endl;
		++tab;
		n.for_each_child([&visitor] (std::any& n) { any_tree::visit_node(visitor, n); });
	}),
	any_tree::make_child_visitor<function_node>([&visitor, &tab] (function_node& n) { 
		tabs(tab); 
		std::cout << "func " << n.payload().name << std::endl;
		++tab;
		for(const auto& param: n.payload().params) {
		    tabs(tab);
		    std::cout << param.name << ' ' << param.tid << std::endl;
		}
		tabs(--tab);
		std::cout << n.payload().return_type << std::endl;
		++tab;
		n.for_each_child([&visitor] (std::any& n) { any_tree::visit_node(visitor, n); });
		--tab;
	}),
	any_tree::make_child_visitor<statement_node>([&visitor, &tab] (statement_node& n) {
		tabs(tab);
		std::cout << (n.payload().is_return ? "return " : "") << "statement" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(0));
		--tab;
	}),
	any_tree::make_child_visitor<binary_expr_node>([&visitor, &tab] (binary_expr_node& n) {
		tabs(tab);
		std::cout << "binary expr; op = " << n.payload().oper << std::endl;
		tabs(tab);
		std::cout << "lhs" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(0));
		tabs(--tab);
		std::cout << "rhs" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(1));
		--tab;

	}),
	any_tree::make_child_visitor<identifier_node>([&tab] (identifier_node& n) {
		tabs(tab);
		std::cout << "identifier " << n.payload() << std::endl;
	}),
	any_tree::make_child_visitor<call_node>([&visitor, &tab] (call_node& n) {
		tabs(tab);
		std::cout << "call " << n.payload().callee << std::endl;
		++tab;
		n.for_each_child([&visitor] (std::any& n) { any_tree::visit_node(visitor, n); });
		--tab;
	}),

	any_tree::make_child_visitor<integer_literal_node>([&tab] (integer_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload() << std::endl;
	}),
	any_tree::make_child_visitor<floating_literal_node>([&tab] (floating_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload() << std::endl;
	}),
	any_tree::make_child_visitor<char_literal_node>([&tab] (char_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload() << std::endl;
	}),
	any_tree::make_child_visitor<string_literal_node>([&tab] (string_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload() << std::endl;
	}),
	any_tree::make_child_visitor<bool_literal_node>([&tab] (bool_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload() << std::endl;
	}),
    };


    any_tree::visit_node(visitor, tree);



    return 0;
}
