#include <utility>

#include "functions.hpp"


auto special_functions::cast_exists(type::type_id from_t, type::type_id to_t) const noexcept -> bool {
    const auto iter = _casts.find(std::make_pair(from_t, to_t));
    return iter != _casts.end();
}

auto special_functions::cast(type::type_id from_t, type::type_id to_t) const noexcept -> cast_inserter_wrapper {
    if(const auto iter = _casts.find(std::make_pair(from_t, to_t)); iter != _casts.end()) {
	return iter->second;
    }
    return {};
}

void special_functions::insert_cast(type::type_id from_t, type::type_id to_t, cast_inserter_wrapper inserter) noexcept {
    _casts[std::make_pair(from_t, to_t)] = std::move(inserter);
}

auto special_functions::unary_exists(const std::string& oper, type::type_id tid) const noexcept -> bool {
    if(auto iter = _unary.find(oper); iter != _unary.end()) {
	const auto& op_storage = iter->second;
	const auto op_iter = op_storage.find(tid);
	return op_iter != op_storage.end();
    }
    return false;
}

auto special_functions::unary(const std::string& oper, type::type_id tid) const noexcept -> unary_inserter_wrapper {
    if(auto iter = _unary.find(oper); iter != _unary.end()) {
	const auto& op_storage = iter->second;
	if(const auto op_iter = op_storage.find(tid); op_iter != op_storage.end()) {
	    return op_iter->second;
	}
    }
    return {};
}

void special_functions::insert_unary(const std::string& oper, type::type_id tid, unary_inserter_wrapper inserter) noexcept {
    _unary[oper][tid] = std::move(inserter);
}

auto special_functions::binary_exists(const std::string& oper, type::type_id left, type::type_id right) const noexcept -> bool {
    if(auto iter = _binary.find(oper); iter != _binary.end()) {
	const auto& op_storage = iter->second;
	const auto op_iter = op_storage.find(std::make_pair(left, right));
	return op_iter != op_storage.end();
    }
    return false;
}

auto special_functions::binary(const std::string& oper, type::type_id left, type::type_id right) const noexcept -> binary_inserter_wrapper {
    if(auto iter = _binary.find(oper); iter != _binary.end()) {
	const auto& op_storage = iter->second;
	if(const auto op_iter = op_storage.find(std::make_pair(left, right)); op_iter != op_storage.end()) {
	    return op_iter->second;
	}
    }
    return {};
}

void special_functions::insert_binary(const std::string& oper, type::type_id left, type::type_id right, binary_inserter_wrapper inserter) noexcept {
    _binary[oper][std::make_pair(left, right)] = std::move(inserter);
}
