#include <memory>
#include <utility>

#include "type/registry.hpp"
#include "type/type_id.hpp"
#include "type/type.hpp"
#include "type/array_type.hpp"
#include "type/anon_struct_type.hpp"
#include "type/struct_type.hpp"
#include "type/function_type.hpp"


auto type::registry::id(const std::string& name) const noexcept -> type_id {
    if(auto iter = _names.find(name); iter != _names.end()) {
	return iter->second;
    }
    return {};
}

auto type::registry::id(const std::vector<type_id>& members) noexcept -> type_id {
    if(auto iter = _anon_struct_ids.find(members); iter != _anon_struct_ids.end()) {
	return iter->second;
    }

    std::vector<llvm::Type*> members_llvm(members.size());
    std::ranges::transform(members, members_llvm.begin(), [this] (auto type) { return *get(type).value_or(::type::type{}); });

    llvm::StructType* anon_struct_t = llvm::StructType::get(*_context, members_llvm);

    type_id tid{next_id()};

    _anon_struct_ids[members] = tid;
    _anon_structs.try_emplace(tid, anon_struct_t, members);

    return tid;
}

auto type::registry::id(const std::vector<type_id>& params, type_id ret) noexcept -> type_id {
    auto func_pair = std::make_pair(params, ret);
    if(auto iter = _function_ids.find(func_pair); iter != _function_ids.end()) {
	return iter->second;
    }

    std::vector<llvm::Type*> params_llvm(params.size());
    std::ranges::transform(params, params_llvm.begin(), [this] (auto type) { return *get(type).value_or(::type::type{}); });

    llvm::FunctionType* function_t = llvm::FunctionType::get(*get(ret).value_or(type{}), params_llvm, false);

    type_id fid{next_id()};

    _function_ids[func_pair] = fid;
    _functions.try_emplace(fid, function_t, params, ret);

    return fid;
}

auto type::registry::id(type_id elements, std::size_t size) noexcept -> type_id {
    auto arr_pair = std::make_pair(elements, size);
    if(auto iter = _array_ids.find(arr_pair); iter != _array_ids.end()) {
	return iter->second;
    }

    llvm::ArrayType* array_t = llvm::ArrayType::get(*get(elements).value_or(type{}), size);

    type_id aid{next_id()};

    _array_ids[arr_pair] = aid;
    _arrays.try_emplace(aid, array_t, elements, size);

    return aid;
}

auto type::registry::get(type_id tid) const noexcept -> std::optional<type> {
    if(auto iter = _primitives.find(tid); iter != _primitives.end()) {
	return iter->second;
    }

    if (const struct_type *struct_t = get_struct(tid); struct_t != nullptr) {
        return type{*struct_t};
    }

    if (const anon_struct_type *anon_struct_t = get_anon_struct(tid); anon_struct_t != nullptr) {
        return type{*anon_struct_t};
    }

    if (const function_type *function_t = get_function(tid); function_t != nullptr) {
        return type{*function_t};
    }

    if (const array_type *array_t = get_array(tid); array_t != nullptr) {
        return type{*array_t};
    }

    return {};
}

auto type::registry::get_struct(type_id tid) const noexcept -> const struct_type* {
    if(auto iter = _structs.find(tid); iter != _structs.end()) {
	return &iter->second;
    }
    return nullptr;
}

auto type::registry::get_anon_struct(type_id tid) const noexcept -> const anon_struct_type* {
    if(auto iter = _anon_structs.find(tid); iter != _anon_structs.end()) {
	return &iter->second;
    }
    return nullptr;
}

auto type::registry::get_function(type_id tid) const noexcept -> const function_type* {
    if(auto iter = _functions.find(tid); iter != _functions.end()) {
	return &iter->second;
    }
    return nullptr;
}

auto type::registry::get_array(type_id tid) const noexcept -> const array_type* {
    if(auto iter = _arrays.find(tid); iter != _arrays.end()) {
	return &iter->second;
    }
    return nullptr;
}

auto type::registry::get(const std::string& name) const noexcept -> std::optional<type> {
    return get(id(name));
}

auto type::registry::get_struct(const std::string& name) const noexcept -> const struct_type* {
    return get_struct(id(name));
}

auto type::registry::get_function(const std::string& name) const noexcept -> const function_type* {
    return get_function(id(name));
}

auto type::registry::get_array(const std::string& name) const noexcept -> const array_type* {
    return get_array(id(name));
}

auto type::registry::make_struct(const std::string& name, const struct_type::members_type& members) noexcept -> const struct_type& {
    std::vector<llvm::Type*> members_llvm(members.size());
    std::ranges::transform(members, members_llvm.begin(), [this] (auto type) { return *get(type.second).value_or(::type::type{}); });

    llvm::StructType* struct_t = llvm::StructType::get(*_context, members_llvm);
    struct_t->setName(name);

    type_id sid{next_id()};

    _names[name] = sid;
    _structs.try_emplace(sid, struct_t, members);

    return _structs.find(sid)->second;
}

auto type::registry::make_anon_struct(const std::vector<type_id>& members) noexcept -> const anon_struct_type& {
    return *get_anon_struct(id(members));
}

auto type::registry::make_function(const std::vector<type_id>& params, type_id ret) noexcept -> const function_type& {
    return *get_function(id(params, ret));
}

auto type::registry::make_array(type_id elements, std::size_t size) noexcept -> const array_type& {
    return *get_array(id(elements, size));
}

void type::registry::make_primitives() noexcept {
    _primitives.try_emplace(type_id::undetermined);
    _primitives.try_emplace(type_id::void_, llvm::Type::getVoidTy  (*_context));
    _primitives.try_emplace(type_id::bool_, llvm::Type::getInt1Ty  (*_context));
    _primitives.try_emplace(type_id::char_, llvm::Type::getInt8Ty  (*_context));
    _primitives.try_emplace(type_id::u8,    llvm::Type::getInt8Ty  (*_context));
    _primitives.try_emplace(type_id::u16,   llvm::Type::getInt16Ty (*_context));
    _primitives.try_emplace(type_id::u32,   llvm::Type::getInt32Ty (*_context));
    _primitives.try_emplace(type_id::u64,   llvm::Type::getInt64Ty (*_context));
    _primitives.try_emplace(type_id::i8,    llvm::Type::getInt8Ty  (*_context));
    _primitives.try_emplace(type_id::i16,   llvm::Type::getInt16Ty (*_context));
    _primitives.try_emplace(type_id::i32,   llvm::Type::getInt32Ty (*_context));
    _primitives.try_emplace(type_id::i64,   llvm::Type::getInt64Ty (*_context));
    _primitives.try_emplace(type_id::fp32,  llvm::Type::getFloatTy (*_context));
    _primitives.try_emplace(type_id::fp64,  llvm::Type::getDoubleTy(*_context));
}
