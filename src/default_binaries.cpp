#include "functions.hpp"
#include "type/type_id.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>


void ui_addition(special_functions& functions) {
    auto binary = [] (llvm::IRBuilderBase* builder, llvm::Value* lhs, llvm::Value* rhs) {
	return builder->CreateAdd(lhs, rhs, "add");
    };

    auto add_binary = [&functions, &binary] (type::type_id type) {
	functions.binary("+").insert(type, type, type);
	functions.binary("+").specialize(type, type, binary);
    };

    add_binary(type::type_id::u8 );
    add_binary(type::type_id::u16);
    add_binary(type::type_id::u32);
    add_binary(type::type_id::u64);

    add_binary(type::type_id::i8 );
    add_binary(type::type_id::i16);
    add_binary(type::type_id::i32);
    add_binary(type::type_id::i64);
}

void fp_addition(special_functions& functions) {
    auto binary = [] (llvm::IRBuilderBase* builder, llvm::Value* lhs, llvm::Value* rhs) {
	return builder->CreateFAdd(lhs, rhs, "add");
    };

    auto add_binary = [&functions, &binary] (type::type_id type) {
	functions.binary("+").insert(type, type, type);
	functions.binary("+").specialize(type, type, binary);
    };

    add_binary(type::type_id::fp32);
    add_binary(type::type_id::fp64);
}

void ui_subtruction(special_functions& functions) {
    auto binary = [] (llvm::IRBuilderBase* builder, llvm::Value* lhs, llvm::Value* rhs) {
	return builder->CreateSub(lhs, rhs, "sub");
    };

    auto add_binary = [&functions, &binary] (type::type_id type) {
	functions.binary("-").insert(type, type, type);
	functions.binary("-").specialize(type, type, binary);
    };

    add_binary(type::type_id::u8 );
    add_binary(type::type_id::u16);
    add_binary(type::type_id::u32);
    add_binary(type::type_id::u64);

    add_binary(type::type_id::i8 );
    add_binary(type::type_id::i16);
    add_binary(type::type_id::i32);
    add_binary(type::type_id::i64);
}

void fp_subtruction(special_functions& functions) {
    auto binary = [] (llvm::IRBuilderBase* builder, llvm::Value* lhs, llvm::Value* rhs) {
	return builder->CreateFSub(lhs, rhs, "sub");
    };

    auto add_binary = [&functions, &binary] (type::type_id type) {
	functions.binary("-").insert(type, type, type);
	functions.binary("-").specialize(type, type, binary);
    };

    add_binary(type::type_id::fp32);
    add_binary(type::type_id::fp64);
}

void ui_multiplication(special_functions& functions) {
    auto binary = [] (llvm::IRBuilderBase* builder, llvm::Value* lhs, llvm::Value* rhs) {
	return builder->CreateMul(lhs, rhs, "mul");
    };

    auto add_binary = [&functions, &binary] (type::type_id type) {
	functions.binary("*").insert(type, type, type);
	functions.binary("*").specialize(type, type, binary);
    };

    add_binary(type::type_id::u8 );
    add_binary(type::type_id::u16);
    add_binary(type::type_id::u32);
    add_binary(type::type_id::u64);

    add_binary(type::type_id::i8 );
    add_binary(type::type_id::i16);
    add_binary(type::type_id::i32);
    add_binary(type::type_id::i64);
}

void fp_multiplication(special_functions& functions) {
    auto binary = [] (llvm::IRBuilderBase* builder, llvm::Value* lhs, llvm::Value* rhs) {
	return builder->CreateFMul(lhs, rhs, "mul");
    };

    auto add_binary = [&functions, &binary] (type::type_id type) {
	functions.binary("*").insert(type, type, type);
	functions.binary("*").specialize(type, type, binary);
    };

    add_binary(type::type_id::fp32);
    add_binary(type::type_id::fp64);
}

void u_division(special_functions& functions) {
    auto binary = [] (llvm::IRBuilderBase* builder, llvm::Value* lhs, llvm::Value* rhs) {
	return builder->CreateUDiv(lhs, rhs, "div");
    };

    auto add_binary = [&functions, &binary] (type::type_id type) {
	functions.binary("/").insert(type, type, type);
	functions.binary("/").specialize(type, type, binary);
    };

    add_binary(type::type_id::u8 );
    add_binary(type::type_id::u16);
    add_binary(type::type_id::u32);
    add_binary(type::type_id::u64);
}

void i_division(special_functions& functions) {
    auto binary = [] (llvm::IRBuilderBase* builder, llvm::Value* lhs, llvm::Value* rhs) {
	return builder->CreateSDiv(lhs, rhs, "div");
    };

    auto add_binary = [&functions, &binary] (type::type_id type) {
	functions.binary("/").insert(type, type, type);
	functions.binary("/").specialize(type, type, binary);
    };

    add_binary(type::type_id::i8 );
    add_binary(type::type_id::i16);
    add_binary(type::type_id::i32);
    add_binary(type::type_id::i64);
}

void fp_division(special_functions& functions) {
    auto binary = [] (llvm::IRBuilderBase* builder, llvm::Value* lhs, llvm::Value* rhs) {
	return builder->CreateFDiv(lhs, rhs, "div");
    };

    auto add_binary = [&functions, &binary] (type::type_id type) {
	functions.binary("/").insert(type, type, type);
	functions.binary("/").specialize(type, type, binary);
    };

    add_binary(type::type_id::fp32);
    add_binary(type::type_id::fp64);
}

void default_binaries(special_functions& functions) {
    functions.new_binary("+", 0);
    functions.new_binary("-", 0);
    functions.new_binary("*", 1);
    functions.new_binary("/", 1);

    ui_addition(functions);
    fp_addition(functions);

    ui_subtruction(functions);
    fp_subtruction(functions);

    ui_multiplication(functions);
    fp_multiplication(functions);

    u_division(functions);
    i_division(functions);
    fp_division(functions);
}
