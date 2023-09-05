#include <memory>
#include <utility>

#include "type/registry.hpp"
#include "type/type_id.hpp"
#include "type/type.hpp"
#include "type/array_type.hpp"
#include "type/anon_struct_type.hpp"
#include "type/struct_type.hpp"
#include "type/function_type.hpp"


auto type::registry::id(const std::vector<type_id>& members) noexcept -> type_id {
    if(auto iter = _anon_structs.find(members); iter != _anon_structs.end()) {
	return iter->second;
    }

    std::vector<llvm::Type*> members_llvm(members.size());
    std::ranges::transform(members, members_llvm.begin(), [this] (auto type) { return *get(type); });

    llvm::StructType* anon_struct_t = llvm::StructType::get(_context, members_llvm);

    type_id tid{next_id()};

    _ids[tid] = std::make_unique<anon_struct_type>(anon_struct_t, members);
    _anon_structs[members] = tid;

    return tid;
}

auto type::registry::id(const std::vector<type_id>& params, type_id ret) noexcept -> type_id {
    auto func_pair = std::make_pair(params, ret);
    if(auto iter = _functions.find(func_pair); iter != _functions.end()) {
	return iter->second;
    }

    std::vector<llvm::Type*> params_llvm(params.size());
    std::ranges::transform(params, params_llvm.begin(), [this] (auto type) { return *get(type); });

    llvm::FunctionType* function_t = llvm::FunctionType::get(*get(ret), params_llvm, false);

    type_id fid{next_id()};

    _ids[fid] = std::make_unique<function_type>(function_t, params, ret);
    _functions[func_pair] = fid;

    return fid;
}

auto type::registry::id(type_id elements, std::size_t size) noexcept -> type_id {
    auto arr_pair = std::make_pair(elements, size);
    if(auto iter = _arrays.find(arr_pair); iter != _arrays.end()) {
	return iter->second;
    }

    llvm::ArrayType* array_t = llvm::ArrayType::get(*get(elements), size);

    type_id aid{next_id()};

    _ids[aid] = std::make_unique<array_type>(array_t, elements, size);
    _arrays[arr_pair] = aid;

    return aid;
}

auto type::registry::get_struct(const std::string& name) noexcept -> struct_type& {
    return static_cast<struct_type&>(get(name));
}

auto type::registry::get_anon_struct(const std::vector<type_id>& members) noexcept -> anon_struct_type& {
    return static_cast<anon_struct_type&>(get(id(members)));
}

auto type::registry::get_function(const std::vector<type_id>& params, type_id ret) noexcept -> function_type& {
    return static_cast<function_type&>(get(id(params, ret)));
}

auto type::registry::get_array(type_id elements, std::size_t size) noexcept -> array_type& {
    return static_cast<array_type&>(get(id(elements, size)));
}

auto type::registry::get_struct(type_id tid) noexcept -> struct_type& {
    return static_cast<struct_type&>(get(tid));
}

auto type::registry::get_anon_struct(type_id tid) noexcept -> anon_struct_type& {
    return static_cast<anon_struct_type&>(get(tid));
}

auto type::registry::get_function(type_id tid) noexcept -> function_type& {
    return static_cast<function_type&>(get(tid));
}

auto type::registry::get_array(type_id tid) noexcept -> array_type& {
    return static_cast<array_type&>(get(tid));
}

auto type::registry::make_struct(const std::string& name, const std::vector<std::pair<std::string, type_id>>& members) noexcept -> type_id {
    std::vector<llvm::Type*> members_llvm(members.size());
    std::ranges::transform(members, members_llvm.begin(), [this] (auto type) { return *get(type.second); });

    llvm::StructType* struct_t = llvm::StructType::get(_context, members_llvm);
    struct_t->setName(name);

    type_id sid{next_id()};

    _ids[sid] = std::make_unique<struct_type>(struct_t, members);
    _names[name] = sid;

    return sid;
}

auto type::registry::is_struct(type_id tid) noexcept -> bool {
    if(const type& type = get(tid); *type) {
	return type->isStructTy();
    }
    return false;
}

auto type::registry::is_array(type_id tid) noexcept -> bool {
    if(const type& type = get(tid); *type) {
	return type->isArrayTy();
    }
    return false;
}

auto type::registry::is_function(type_id tid) noexcept -> bool {
    if(const type& type = get(tid); *type) {
	return type->isFunctionTy();
    }
    return false;
}

void type::registry::make_primitives() noexcept {
    _ids[type_id::undetermined] = std::make_unique<type>();
    _ids[type_id::void_]        = std::make_unique<type>(llvm::Type::getVoidTy(_context));
    _ids[type_id::bool_]        = std::make_unique<type>(llvm::Type::getInt1Ty(_context));
    _ids[type_id::char_]        = std::make_unique<type>(llvm::Type::getInt8Ty(_context));
    _ids[type_id::u8]           = std::make_unique<type>(llvm::Type::getInt8Ty(_context));
    _ids[type_id::u16]          = std::make_unique<type>(llvm::Type::getInt16Ty(_context));
    _ids[type_id::u32]          = std::make_unique<type>(llvm::Type::getInt32Ty(_context));
    _ids[type_id::u64]          = std::make_unique<type>(llvm::Type::getInt64Ty(_context));
    _ids[type_id::i8]           = std::make_unique<type>(llvm::Type::getInt8Ty(_context));
    _ids[type_id::i16]          = std::make_unique<type>(llvm::Type::getInt16Ty(_context));
    _ids[type_id::i32]          = std::make_unique<type>(llvm::Type::getInt32Ty(_context));
    _ids[type_id::i64]          = std::make_unique<type>(llvm::Type::getInt64Ty(_context));
    _ids[type_id::fp32]         = std::make_unique<type>(llvm::Type::getFloatTy(_context));
    _ids[type_id::fp64]         = std::make_unique<type>(llvm::Type::getDoubleTy(_context));
}
