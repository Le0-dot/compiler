#pragma once

#include <string>
#include <vector>
#include <ranges>
#include <optional>
#include <type_traits>
#include <unordered_map>

#include "type/type_id.hpp"


template<typename T>
requires std::is_trivially_copy_constructible_v<T>
class scope {
    std::unordered_map<std::string, T> _symbols{};

public:
    auto get(const std::string& name) noexcept -> std::optional<T> {
	if(auto iter = _symbols.find(name); iter != _symbols.end()) {
	    return iter->second;
	}
	return {};
    }

    void add(const std::string& name, T value) noexcept {
	_symbols[name] = value;
    }
};

template<typename T>
class scope_manager {
    std::vector<scope<T>> _scopes{};

public:
    scope_manager() { push(); }

    void push() noexcept { _scopes.emplace_back(); }
    void pop() noexcept { _scopes.pop_back(); }

    auto get(const std::string& name) noexcept -> std::optional<T> {
	for(auto& scope : _scopes | std::views::reverse) {
	    if(auto value = scope.get(name); value.has_value()) {
		return value;
	    }
	}
	return {};
    }

    void add(const std::string& name, T value) noexcept {
	_scopes.back().add(name, value);
    }
};

template<typename T>
class scope_pusher {
    scope_manager<T>* _scope;

public:
    explicit scope_pusher(scope_manager<T>* scope) : _scope{scope} {
	_scope->push();
    }

    ~scope_pusher() { _scope->pop(); }

    scope_pusher()                         = delete;
    scope_pusher(const scope_pusher<T>&)   = delete;
    scope_pusher(scope_pusher<T>&&)        = delete;
    auto operator=(const scope_pusher<T>&) = delete;
    auto operator=(scope_pusher<T>&&)      = delete;
};
