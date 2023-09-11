#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "functions.hpp"


void literal_casts(special_functions& functions) {
    auto add_cast = [&functions] (type::type_id from_type, type::type_id to_type) {
	functions.cast(from_type).insert(to_type);
    };

    add_cast(type::type_id::u_literal, type::type_id::u8 );
    add_cast(type::type_id::u_literal, type::type_id::u16);
    add_cast(type::type_id::u_literal, type::type_id::u32);
    add_cast(type::type_id::u_literal, type::type_id::u64);

    add_cast(type::type_id::u_literal, type::type_id::i8 );
    add_cast(type::type_id::u_literal, type::type_id::i16);
    add_cast(type::type_id::u_literal, type::type_id::i32);
    add_cast(type::type_id::u_literal, type::type_id::i64);

    add_cast(type::type_id::u_literal, type::type_id::fp32);
    add_cast(type::type_id::u_literal, type::type_id::fp64);

    add_cast(type::type_id::fp_literal, type::type_id::fp32);
    add_cast(type::type_id::fp_literal, type::type_id::fp64);
}

void u_casts(special_functions& functions, type::registry& types) {
    auto get_cast = [] (llvm::Type* to_type, std::string_view name) {
	return [to_type, name] (llvm::IRBuilderBase* builder, llvm::Value* value) { 
	    return builder->CreateIntCast(value, to_type, false, name); 
	};
    };

    auto add_cast = [&functions, &types, &get_cast] (type::type_id from_type, type::type_id to_type) {
	functions.cast(from_type).specialize(to_type, get_cast(**types.get(to_type), "cast"));
    };

    add_cast(type::type_id::bool_, type::type_id::u8 );
    add_cast(type::type_id::bool_, type::type_id::u16);
    add_cast(type::type_id::bool_, type::type_id::u32);
    add_cast(type::type_id::bool_, type::type_id::u64);

    add_cast(type::type_id::bool_, type::type_id::i8 );
    add_cast(type::type_id::bool_, type::type_id::i16);
    add_cast(type::type_id::bool_, type::type_id::i32);
    add_cast(type::type_id::bool_, type::type_id::i64);

    add_cast(type::type_id::char_, type::type_id::u8 );
    add_cast(type::type_id::char_, type::type_id::u16);
    add_cast(type::type_id::char_, type::type_id::u32);
    add_cast(type::type_id::char_, type::type_id::u64);

    add_cast(type::type_id::char_, type::type_id::i8 );
    add_cast(type::type_id::char_, type::type_id::i16);
    add_cast(type::type_id::char_, type::type_id::i32);
    add_cast(type::type_id::char_, type::type_id::i64);

    add_cast(type::type_id::u8,  type::type_id::char_);

    add_cast(type::type_id::u8,  type::type_id::u16);
    add_cast(type::type_id::u8,  type::type_id::u32);
    add_cast(type::type_id::u8,  type::type_id::u64);

    add_cast(type::type_id::u8,  type::type_id::i16);
    add_cast(type::type_id::u8,  type::type_id::i32);
    add_cast(type::type_id::u8,  type::type_id::i64);

    add_cast(type::type_id::u16, type::type_id::u32);
    add_cast(type::type_id::u16, type::type_id::u64);

    add_cast(type::type_id::u16, type::type_id::i32);
    add_cast(type::type_id::u16, type::type_id::i64);

    add_cast(type::type_id::u32, type::type_id::u64);

    add_cast(type::type_id::u32, type::type_id::i64);
}

void i_casts(special_functions& functions, type::registry& types) {
    auto get_cast = [] (llvm::Type* to_type, std::string_view name) {
	return [to_type, name] (llvm::IRBuilderBase* builder, llvm::Value* value) { 
	    return builder->CreateIntCast(value, to_type, true, name); 
	};
    };

    auto add_cast = [&functions, &types, &get_cast] (type::type_id from_type, type::type_id to_type) {
	functions.cast(from_type).specialize(to_type, get_cast(**types.get(to_type), "cast"));
    };

    add_cast(type::type_id::i8,  type::type_id::i16);
    add_cast(type::type_id::i8,  type::type_id::i32);
    add_cast(type::type_id::i8,  type::type_id::i64);

    add_cast(type::type_id::i16, type::type_id::i32);
    add_cast(type::type_id::i16, type::type_id::i64);

    add_cast(type::type_id::i32, type::type_id::i64);
}

void fp_casts(special_functions& functions, type::registry& types) {
    auto get_cast = [] (llvm::Type* to_type, std::string_view name) {
	return [to_type, name] (llvm::IRBuilderBase* builder, llvm::Value* value) { 
	    return builder->CreateFPExt(value, to_type, name); 
	};
    };

    auto add_cast = [&functions, &types, &get_cast] (type::type_id from_type, type::type_id to_type) {
	functions.cast(from_type).specialize(to_type, get_cast(**types.get(to_type), "cast"));
    };

    add_cast(type::type_id::fp32, type::type_id::fp64);
}

void ui_bool_casts(special_functions& functions, type::registry& types) {
    auto get_cast = [] (llvm::Type* from_type, std::string_view name) {
	return [from_type, name] (llvm::IRBuilderBase* builder, llvm::Value* value) { 
	    return builder->CreateICmpNE(value, llvm::ConstantInt::get(from_type, 0), name); 
	};
    };

    auto add_cast = [&functions, &types, &get_cast] (type::type_id from_type, type::type_id to_type) {
	functions.cast(from_type).specialize(to_type, get_cast(**types.get(from_type), "cast"));
    };

    add_cast(type::type_id::u8,  type::type_id::bool_);
    add_cast(type::type_id::u16, type::type_id::bool_);
    add_cast(type::type_id::u32, type::type_id::bool_);
    add_cast(type::type_id::u64, type::type_id::bool_);

    add_cast(type::type_id::i8,  type::type_id::bool_);
    add_cast(type::type_id::i16, type::type_id::bool_);
    add_cast(type::type_id::i32, type::type_id::bool_);
    add_cast(type::type_id::i64, type::type_id::bool_);
}

void fp_bool_casts(special_functions& functions, type::registry& types) {
    auto get_cast = [] (llvm::Type* from_type, std::string_view name) {
	return [from_type, name] (llvm::IRBuilderBase* builder, llvm::Value* value) { 
	    return builder->CreateFCmpONE(value, llvm::ConstantFP::get(from_type, 0), name); 
	};
    };

    auto add_cast = [&functions, &types, &get_cast] (type::type_id from_type, type::type_id to_type) {
	functions.cast(from_type).specialize(to_type, get_cast(**types.get(from_type), "cast"));
    };

    add_cast(type::type_id::fp32, type::type_id::bool_);
    add_cast(type::type_id::fp64, type::type_id::bool_);
}

void u_fp_casts(special_functions& functions, type::registry& types) {
    auto get_cast = [] (llvm::Type* to_type, std::string_view name) {
	return [to_type, name] (llvm::IRBuilderBase* builder, llvm::Value* value) { 
	    return builder->CreateUIToFP(value, to_type, name); 
	};
    };

    auto add_cast = [&functions, &types, &get_cast] (type::type_id from_type, type::type_id to_type) {
	functions.cast(from_type).specialize(to_type, get_cast(**types.get(to_type), "cast"));
    };

    add_cast(type::type_id::bool_, type::type_id::fp32);
    add_cast(type::type_id::bool_, type::type_id::fp64);

    add_cast(type::type_id::u8,  type::type_id::fp32);
    add_cast(type::type_id::u8,  type::type_id::fp64);

    add_cast(type::type_id::u16, type::type_id::fp32);
    add_cast(type::type_id::u16, type::type_id::fp64);

    add_cast(type::type_id::u32, type::type_id::fp32);
    add_cast(type::type_id::u32, type::type_id::fp64);

    add_cast(type::type_id::u64, type::type_id::fp32);
    add_cast(type::type_id::u64, type::type_id::fp64);
}

void i_fp_casts(special_functions& functions, type::registry& types) {
    auto get_cast = [] (llvm::Type* to_type, std::string_view name) {
	return [to_type, name] (llvm::IRBuilderBase* builder, llvm::Value* value) { 
	    return builder->CreateSIToFP(value, to_type, name); 
	};
    };

    auto add_cast = [&functions, &types, &get_cast] (type::type_id from_type, type::type_id to_type) {
	functions.cast(from_type).specialize(to_type, get_cast(**types.get(to_type), "cast"));
    };

    add_cast(type::type_id::i8,  type::type_id::fp32);
    add_cast(type::type_id::i8,  type::type_id::fp64);

    add_cast(type::type_id::i16, type::type_id::fp32);
    add_cast(type::type_id::i16, type::type_id::fp64);

    add_cast(type::type_id::i32, type::type_id::fp32);
    add_cast(type::type_id::i32, type::type_id::fp64);

    add_cast(type::type_id::i64, type::type_id::fp32);
    add_cast(type::type_id::i64, type::type_id::fp64);
}


void default_casts(special_functions& functions, type::registry& types) {
    literal_casts(functions);
    i_casts(functions, types);
    u_casts(functions, types);
    fp_casts(functions, types);
    ui_bool_casts(functions, types);
    fp_bool_casts(functions, types);
    u_fp_casts(functions, types);
    i_fp_casts(functions, types);
}
