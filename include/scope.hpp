#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "type/type_id.hpp"


class scope {
    std::unordered_map<std::string, type::type_id> _symbols{};

public:
    auto get(const std::string& name) noexcept -> type::type_id;
    void add(const std::string& name, type::type_id tid) noexcept;
};

class scope_manager {
    std::vector<scope> _scopes{};

public:
    void push() noexcept;
    void pop() noexcept;

    auto get(const std::string& name) noexcept -> type::type_id;
    void add(const std::string& name, type::type_id tid) noexcept;
};
