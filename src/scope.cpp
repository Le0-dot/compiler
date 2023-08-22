#include "scope.hpp"

#include <ranges>

#include "type/type_id.hpp"


auto scope::get(const std::string& name) noexcept -> type::type_id {
    return _symbols[name];
}

void scope::add(const std::string& name, type::type_id tid) noexcept {
    _symbols[name] = tid;
}

void scope_manager::push() noexcept {
    _scopes.emplace_back();
}

void scope_manager::pop() noexcept {
    _scopes.pop_back();
}

auto scope_manager::get(const std::string& name) noexcept -> type::type_id {
    for(auto& scope : _scopes | std::views::reverse) {
	if(auto tid = scope.get(name); valid(tid)) {
	    return tid;
	}
    }
    return type::type_id{};
}

void scope_manager::add(const std::string& name, type::type_id tid) noexcept {
    _scopes.back().add(name, tid);
}
