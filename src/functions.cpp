#include <utility>

#include "functions.hpp"
#include "type/type_id.hpp"


auto casts::get(type::type_id to_type) const -> std::optional<inserter_wrapper> {
    if(auto iter = _casts.find(to_type); iter != _casts.end()) {
	return iter->second;
    }
    return {};
}

void casts::insert(type::type_id to_type) {
    _casts[to_type] = {};
}

void casts::specialize(type::type_id to_type, inserter_wrapper inserter) {
    _casts[to_type] = std::move(inserter);
}

auto unary_operator::get(type::type_id operand_type) -> operator_info {
    if(auto iter = _unary.find(operand_type); iter != _unary.end()) {
	return iter->second;
    }
    return {};
}

void unary_operator::insert(type::type_id operand_type, type::type_id return_type) {
    _unary[operand_type] = {return_type, {}};
}

void unary_operator::specialize(type::type_id operand_type, inserter_wrapper inserter) {
    _unary[operand_type].second = std::move(inserter);
}

auto binary_operator::get(type::type_id left, type::type_id right) -> operator_info {
    if(auto iter = _binary.find(std::make_pair(left, right)); iter != _binary.end()) {
	return iter->second;
    }
    return {};
}

void binary_operator::insert(type::type_id left, type::type_id right, type::type_id return_type) {
    _binary[std::make_pair(left, right)] = {return_type, {}};
}

void binary_operator::specialize(type::type_id left, type::type_id right, inserter_wrapper inserter) {
    _binary[std::make_pair(left, right)].second = std::move(inserter);
}

auto special_functions::new_unary(const std::string& oper, std::uint64_t precedense) noexcept -> bool {
    auto& unary_oper = _unary[oper];
    if(unary_oper.precedense() != 0U) {
	return false;
    }

    unary_oper = unary_operator{precedense};

    return true;
}

auto special_functions::new_binary(const std::string& oper, std::uint64_t precedense) noexcept -> bool {
    auto& binary_oper = _binary[oper];
    if(binary_oper.precedense() != 0U) {
	return false;
    }

    binary_oper = binary_operator{precedense};

    return true;
}
