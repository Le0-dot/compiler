#include <format>
#include <iostream>
#include <fstream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <nlohmann/json.hpp>
#include <any_tree.hpp>
#include <ostream>

#include "any_tree/visitor.hpp"
#include "tree.hpp"
#include "type/type_id.hpp"
#include "type/registry.hpp"
#include "functions.hpp"
#include "semantic_analyzer.hpp"
#include "code_generator.hpp"


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
    type::registry types{&context};
    special_functions functions{};

    default_casts(functions, types);
    default_binaries(functions);

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
	any_tree::make_const_child_visitor<return_statement_node>([&visitor, &tab] (const return_statement_node& n) {
		tabs(tab);
		std::cout << "return statement" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(0));
		--tab;
	}),
	any_tree::make_const_child_visitor<let_statement_node>([&visitor, &tab] (const let_statement_node& n) {
		tabs(tab);
		std::cout << "let statement" << std::endl;
		++tab;
		for(const auto& child: n.children()) {
		    any_tree::visit_node(visitor, child);
		}
		--tab;
	}),
	any_tree::make_const_child_visitor<var_def_node>([&visitor, &tab] (const var_def_node& n) {
		tabs(tab);
		std::cout << n.payload().name << " " << n.payload().type << std::endl;
		if(!n.children().empty()) {
		    ++tab;
		    any_tree::visit_node(visitor, n.child_at(0));
		    --tab;
		}
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
	any_tree::make_const_child_visitor<if_node>([&visitor, &tab] (const if_node& n) {
		tabs(tab);
		std::cout << "if stmt" << std::endl; 
		++tab;
		tabs(tab);
		std::cout << "let" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(0));
		--tab;
		tabs(tab);
		std::cout << "cond" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(1));
		--tab;
		tabs(tab);
		std::cout << "then" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(2));
		tab -= 2;
	}),
	any_tree::make_const_child_visitor<if_else_node>([&visitor, &tab] (const if_else_node& n) {
		tabs(tab);
		std::cout << "if stmt" << std::endl; 
		++tab;
		tabs(tab);
		std::cout << "let" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(0));
		--tab;
		tabs(tab);
		std::cout << "cond" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(1));
		--tab;
		tabs(tab);
		std::cout << "then" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(2));
		--tab;
		tabs(tab);
		std::cout << "else" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(3));
		tab -= 2;
	}),
	any_tree::make_const_child_visitor<if_else_expr_node>([&visitor, &tab] (const if_else_expr_node& n) {
		tabs(tab);
		std::cout << "if" << std::endl; 
		++tab;
		tabs(tab);
		std::cout << "let" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(0));
		--tab;
		tabs(tab);
		std::cout << "cond" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(1));
		--tab;
		tabs(tab);
		std::cout << "then" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(2));
		--tab;
		tabs(tab);
		std::cout << "else" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(3));
		tab -= 2;
	}),
	any_tree::make_const_child_visitor<loop_node>([&visitor, &tab] (const loop_node& n) {
		tabs(tab);
		std::cout << "loop" << std::endl;
		++tab;
		tabs(tab);
		std::cout << "let" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(0));
		--tab;
		tabs(tab);
		std::cout << "cond" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(1));
		--tab;
		tabs(tab);
		std::cout << "post" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(2));
		--tab;
		tabs(tab);
		std::cout << "body" << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(3));
		--tab;
	}),
	any_tree::make_const_child_visitor<block_node>([&visitor] (const block_node& n) {
		n.for_each_child([&visitor] (const std::any& n) { any_tree::visit_node(visitor, n); });
	}),
	any_tree::make_const_child_visitor<implicit_cast_node>([&visitor, &tab] (const implicit_cast_node& n) {
		tabs(tab);
		std::cout << "cast from " << n.payload().from_type << " to " << n.payload().to_type << std::endl;
		++tab;
		any_tree::visit_node(visitor, n.child_at(0));
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
		std::cout << "literal " << n.payload().value << ' ' << n.payload().type << std::endl;
	}),
	any_tree::make_const_child_visitor<floating_literal_node>([&tab] (const floating_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload().value << ' ' << n.payload().type << std::endl;
	}),
	any_tree::make_const_child_visitor<char_literal_node>([&tab] (const char_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload().value << ' ' << n.payload().type << std::endl;
	}),
	any_tree::make_const_child_visitor<string_literal_node>([&tab] (const string_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload().value << ' ' << n.payload().type << std::endl;
	}),
	any_tree::make_const_child_visitor<bool_literal_node>([&tab] (const bool_literal_node& n) {
		tabs(tab);
		std::cout << "literal " << n.payload().value << ' ' << n.payload().type << std::endl;
	}),
	any_tree::make_const_child_visitor<void>([] () {
		std::cout << "nothing to see here" << std::endl;
	}),
    };

    std::cout << "building finished" << std::endl;

    semantic_analyzer analyzer{&functions, &types};
    auto analyzer_result = any_tree::visit_node(analyzer.get_visitor(), tree);
    std::cout << analyzer_result << std::endl;

    any_tree::visit_node(visitor, tree);

    if(!type::valid(analyzer_result)) {
	std::cerr << "semantic analyzer pass failed" << std::endl;
	return 1;
    }

    code_generator generator{argv[1], &context, &functions, &types};
    if(llvm::Value* func = any_tree::visit_node(generator.get_visitor(), tree); func == nullptr) {
	std::cerr << "code generator pass failed" << std::endl;
	return 1;
    }

    llvm::Module& module = generator.get_module();
    
    std::cerr << "before optimization" << std::endl;
    module.print(llvm::errs(), nullptr);

    // Create the analysis managers.
    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;

    // Create the new pass manager builder.
    llvm::PassBuilder pass_builder;

    // Register all the basic analyses with the managers.
    pass_builder.registerModuleAnalyses(mam);
    pass_builder.registerCGSCCAnalyses(cgam);
    pass_builder.registerFunctionAnalyses(fam);
    pass_builder.registerLoopAnalyses(lam);
    pass_builder.crossRegisterProxies(lam, fam, cgam, mam);

    // Create the pass manager.
    llvm::ModulePassManager mpm = pass_builder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O1);

    mpm.run(module, mam);

    std::cerr << "after optimization" << std::endl;
    module.print(llvm::errs(), nullptr);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string target_triple = llvm::sys::getDefaultTargetTriple();
    module.setTargetTriple(target_triple);

    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    if(!target) {
	llvm::errs() << error;
	return 1;
    }

    const char *cpu = "generic";
    const char *features = "";

    llvm::TargetOptions opt;
    //auto rm = std::optional<llvm::Reloc::Model>();
    llvm::TargetMachine* target_machine = target->createTargetMachine(target_triple, cpu, features, opt, {});

    module.setDataLayout(target_machine->createDataLayout());

    std::string filename = std::string{argv[1]} + ".o";
    std::error_code error_code;
    llvm::raw_fd_ostream dest(filename, error_code, llvm::sys::fs::OF_None);

    if (error_code) {
	llvm::errs() << "Could not open file: " << error_code.message();
        return 1;
    }
    
    llvm::legacy::PassManager pass;
    auto filetype = llvm::CodeGenFileType::CGFT_ObjectFile;
    if(target_machine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
	std::cerr << "target machine cannot emit files of this type";
	return 1;
    }

    pass.run(module);
    dest.flush();

    std::cout << std::format("Wrote {}\n", filename);

    return 0;
}
