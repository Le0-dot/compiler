#pragma once

#include <string>
#include <utility>
#include <vector>
#include <ranges>
#include <optional>
#include <type_traits>
#include <unordered_map>

#include "type/type_id.hpp"


template<typename T>
using remove_specifiers = std::remove_pointer_t<std::remove_cvref_t<T>>;

template<typename T, typename F = T>
requires std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<F>
class scope {
    std::unordered_map<std::string, T> _symbols{};
    F _function{};

public:
    explicit scope(F function) : _function{function} {}

    auto get(const std::string& name) noexcept -> std::optional<T> {
	if(auto iter = _symbols.find(name); iter != _symbols.end()) {
	    return iter->second;
	}
	return {};
    }

    void add(const std::string& name, T value) noexcept {
	_symbols[name] = value;
    }

    auto function() const noexcept -> F { return _function; }
};

template<typename T, typename F = T>
class scope_manager {
    std::vector<scope<T, F>> _scopes{};

public:
    scope_manager() { push({}); }

    auto begin()       { return _scopes.rbegin(); }
    auto begin() const { return _scopes.rcbegin(); }

    auto end()       { return _scopes.rend(); }
    auto end() const { return _scopes.rcend(); }

    void push(F function) noexcept { _scopes.emplace_back(function); }
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

    auto function() const noexcept -> F { return _scopes.back().function(); }
};

template<typename T, typename F = T>
class scope_pusher {
    scope_manager<T, F>* _scope;

public:
    explicit scope_pusher(scope_manager<T, F>* scope, F function) : _scope{scope} {
	_scope->push(function);
    }

    ~scope_pusher() { _scope->pop(); }

    scope_pusher()                            = delete;
    scope_pusher(const scope_pusher<T, F>&)   = delete;
    scope_pusher(scope_pusher<T, F>&&)        = delete;
    auto operator=(const scope_pusher<T, F>&) = delete;
    auto operator=(scope_pusher<T, F>&&)      = delete;
};
