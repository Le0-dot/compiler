#include <memory>
#include <utility>

#include "types/registry.hpp"
#include "types/type_id.hpp"
#include "types/type.hpp"
#include "types/array_type.hpp"
#include "types/anon_struct_type.hpp"
#include "types/struct_type.hpp"
#include "types/function_type.hpp"


auto type::registry::id(const std::vector<type_id>& members) noexcept -> type_id {
    if(auto it = _anon_structs.find(members); it != _anon_structs.end())
	return it->second;

    std::vector<llvm::Type*> members_llvm(members.size());
    std::ranges::transform(members, members_llvm.begin(), [this] (auto t) { return *get(t); });

    llvm::StructType* st = llvm::StructType::get(_context, members_llvm);

    type_id tid{next_id()};

    _ids[tid] = std::make_unique<anon_struct_type>(st, members);
    _anon_structs[members] = tid;

    return tid;
}

auto type::registry::id(const std::vector<type_id>& params, type_id ret) noexcept -> type_id {
    auto p = std::make_pair(params, ret);
    if(auto it = _functions.find(p); it != _functions.end())
	return it->second;

    std::vector<llvm::Type*> params_llvm(params.size());
    std::ranges::transform(params, params_llvm.begin(), [this] (auto t) { return *get(t); });

    llvm::FunctionType* ft = llvm::FunctionType::get(*get(ret), params_llvm, false);

    type_id fid{next_id()};

    _ids[fid] = std::make_unique<function_type>(ft, params, ret);
    _functions[p] = fid;

    return fid;
}

auto type::registry::id(type_id elements, std::size_t size) noexcept -> type_id {
    auto p = std::make_pair(elements, size);
    if(auto it = _arrays.find(p); it != _arrays.end())
	return it->second;

    llvm::ArrayType* at = llvm::ArrayType::get(*get(elements), size);

    type_id aid{next_id()};

    _ids[aid] = std::make_unique<array_type>(at, elements, size);
    _arrays[p] = aid;

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

auto type::registry::make_struct(const std::string& name, const std::vector<std::pair<std::string, type_id>>& members) noexcept -> type_id {
    std::vector<llvm::Type*> members_llvm(members.size());
    std::ranges::transform(members, members_llvm.begin(), [this] (auto t) { return *get(t.second); });

    llvm::StructType* st = llvm::StructType::get(_context, members_llvm);
    st->setName(name);

    type_id sid{next_id()};

    _ids[sid] = std::make_unique<struct_type>(st, members);
    _names[name] = sid;

    return sid;
}

auto type::registry::is_struct(type_id id) noexcept -> bool {
    if(const type& t = get(id); *t)
	return t->isStructTy();
    return false;
}

auto type::registry::is_array(type_id id) noexcept -> bool {
    if(const type& t = get(id); *t)
	return t->isArrayTy();
    return false;
}

auto type::registry::is_function(type_id id) noexcept -> bool {
    if(const type& t = get(id); *t)
	return t->isFunctionTy();
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
    _ids[type_id::u128]         = std::make_unique<type>(llvm::Type::getInt128Ty(_context));
    _ids[type_id::i8]           = std::make_unique<type>(llvm::Type::getInt8Ty(_context));
    _ids[type_id::i16]          = std::make_unique<type>(llvm::Type::getInt16Ty(_context));
    _ids[type_id::i32]          = std::make_unique<type>(llvm::Type::getInt32Ty(_context));
    _ids[type_id::i64]          = std::make_unique<type>(llvm::Type::getInt64Ty(_context));
    _ids[type_id::i128]         = std::make_unique<type>(llvm::Type::getInt128Ty(_context));
    _ids[type_id::fp32]         = std::make_unique<type>(llvm::Type::getFloatTy(_context));
    _ids[type_id::fp64]         = std::make_unique<type>(llvm::Type::getDoubleTy(_context));
    _ids[type_id::fp128]        = std::make_unique<type>(llvm::Type::getFP128Ty(_context));
}
