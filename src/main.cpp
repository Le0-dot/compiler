#include <iostream>
#include <fstream>

#include <llvm/IR/LLVMContext.h>
#include <nlohmann/json.hpp>
#include <any_tree.hpp>

#include "tree.hpp"
#include "type/type_id.hpp"
#include "type/registry.hpp"
#include "functions.hpp"
#include "semantic_analyzer.hpp"

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

    llvm::LLVMContext context{};
    type::registry types{context};
    special_functions functions{};

    default_casts(functions, types);

    functions.new_binary("+", 0);
    functions.new_binary("-", 0);
    functions.new_binary("*", 1);
    functions.new_binary("/", 1);


    functions.binary("+").insert(type::type_id::u8, type::type_id::u8, type::type_id::u8);
    functions.binary("+").insert(type::type_id::u16, type::type_id::u16, type::type_id::u16);
    functions.binary("+").insert(type::type_id::u32, type::type_id::u32, type::type_id::u32);
    functions.binary("+").insert(type::type_id::u64, type::type_id::u64, type::type_id::u64);

    functions.binary("+").insert(type::type_id::i8, type::type_id::i8, type::type_id::i8);
    functions.binary("+").insert(type::type_id::i16, type::type_id::i16, type::type_id::i16);
    functions.binary("+").insert(type::type_id::i32, type::type_id::i32, type::type_id::i32);
    functions.binary("+").insert(type::type_id::i64, type::type_id::i64, type::type_id::i64);

    functions.binary("+").insert(type::type_id::fp32, type::type_id::fp32, type::type_id::fp32);
    functions.binary("+").insert(type::type_id::fp64, type::type_id::fp64, type::type_id::fp64);


    functions.binary("-").insert(type::type_id::u8, type::type_id::u8, type::type_id::u8);
    functions.binary("-").insert(type::type_id::u16, type::type_id::u16, type::type_id::u16);
    functions.binary("-").insert(type::type_id::u32, type::type_id::u32, type::type_id::u32);
    functions.binary("-").insert(type::type_id::u64, type::type_id::u64, type::type_id::u64);

    functions.binary("-").insert(type::type_id::i8, type::type_id::i8, type::type_id::i8);
    functions.binary("-").insert(type::type_id::i16, type::type_id::i16, type::type_id::i16);
    functions.binary("-").insert(type::type_id::i32, type::type_id::i32, type::type_id::i32);
    functions.binary("-").insert(type::type_id::i64, type::type_id::i64, type::type_id::i64);

    functions.binary("-").insert(type::type_id::fp32, type::type_id::fp32, type::type_id::fp32);
    functions.binary("-").insert(type::type_id::fp64, type::type_id::fp64, type::type_id::fp64);


    functions.binary("*").insert(type::type_id::u8, type::type_id::u8, type::type_id::u8);
    functions.binary("*").insert(type::type_id::u16, type::type_id::u16, type::type_id::u16);
    functions.binary("*").insert(type::type_id::u32, type::type_id::u32, type::type_id::u32);
    functions.binary("*").insert(type::type_id::u64, type::type_id::u64, type::type_id::u64);

    functions.binary("*").insert(type::type_id::i8, type::type_id::i8, type::type_id::i8);
    functions.binary("*").insert(type::type_id::i16, type::type_id::i16, type::type_id::i16);
    functions.binary("*").insert(type::type_id::i32, type::type_id::i32, type::type_id::i32);
    functions.binary("*").insert(type::type_id::i64, type::type_id::i64, type::type_id::i64);

    functions.binary("*").insert(type::type_id::fp32, type::type_id::fp32, type::type_id::fp32);
    functions.binary("*").insert(type::type_id::fp64, type::type_id::fp64, type::type_id::fp64);


    functions.binary("/").insert(type::type_id::u8, type::type_id::u8, type::type_id::u8);
    functions.binary("/").insert(type::type_id::u16, type::type_id::u16, type::type_id::u16);
    functions.binary("/").insert(type::type_id::u32, type::type_id::u32, type::type_id::u32);
    functions.binary("/").insert(type::type_id::u64, type::type_id::u64, type::type_id::u64);

    functions.binary("/").insert(type::type_id::i8, type::type_id::i8, type::type_id::i8);
    functions.binary("/").insert(type::type_id::i16, type::type_id::i16, type::type_id::i16);
    functions.binary("/").insert(type::type_id::i32, type::type_id::i32, type::type_id::i32);
    functions.binary("/").insert(type::type_id::i64, type::type_id::i64, type::type_id::i64);

    functions.binary("/").insert(type::type_id::fp32, type::type_id::fp32, type::type_id::fp32);
    functions.binary("/").insert(type::type_id::fp64, type::type_id::fp64, type::type_id::fp64);

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

    auto tree = tree_builder{&functions, &types}(json);

    std::size_t tab{};
    any_tree::const_children_visitor<void> visitor {
	any_tree::make_const_child_visitor<file_node>([&visitor, &tab] (const file_node& n) { 
		std::cout << "file" << std::endl;
		++tab;
		n.for_each_child([&visitor] (const std::any& n) { any_tree::visit_node(visitor, n); });
	}),
	any_tree::make_const_child_visitor<function_node>([&visitor, &tab] (const function_node& n) { 
		tabs(tab); 
		std::cout << "func " << n.payload().name << std::endl;
		++tab;
		auto param_name = n.payload().params.begin();
		auto param_type = n.payload().params_type.begin();
		while(param_name != n.payload().params.end()) {
		    tabs(tab);
		    std::cout << *param_name++ << ' ' << *param_type++ << std::endl;
		}
		tabs(--tab);
		std::cout << n.payload().return_type << std::endl;
		++tab;
		n.for_each_child([&visitor] (const std::any& n) { any_tree::visit_node(visitor, n); });
		--tab;
	}),
	any_tree::make_const_child_visitor<statement_node>([&visitor, &tab] (const statement_node& n) {
		tabs(tab);
		std::cout << (n.payload().is_return ? "return " : "") << "statement" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(0));
		--tab;
	}),
	any_tree::make_const_child_visitor<binary_expr_node>([&visitor, &tab] (const binary_expr_node& n) {
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
	any_tree::make_const_child_visitor<identifier_node>([&tab] (const identifier_node& n) {
		tabs(tab);
		std::cout << "identifier " << n.payload() << std::endl;
	}),
	any_tree::make_const_child_visitor<call_node>([&visitor, &tab] (const call_node& n) {
		tabs(tab);
		std::cout << "call " << n.payload().callee << std::endl;
		++tab;
		n.for_each_child([&visitor] (const std::any& n) { any_tree::visit_node(visitor, n); });
		--tab;
	}),

	any_tree::make_const_child_visitor<integer_literal_node>([&tab] (const integer_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload().value << std::endl;
	}),
	any_tree::make_const_child_visitor<floating_literal_node>([&tab] (const floating_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload().value << std::endl;
	}),
	any_tree::make_const_child_visitor<char_literal_node>([&tab] (const char_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload().value << std::endl;
	}),
	any_tree::make_const_child_visitor<string_literal_node>([&tab] (const string_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload().value << std::endl;
	}),
	any_tree::make_const_child_visitor<bool_literal_node>([&tab] (const bool_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload().value << std::endl;
	}),
    };

    any_tree::visit_node(visitor, tree);

    semantic_analyzer analyzer{&functions, &types};
    std::cout << any_tree::visit_node(analyzer.get_visitor(), tree) << std::endl;

    return 0;
}
