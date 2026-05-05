#pragma once

#include <utility>

#define DEBUG_MODE

#include "InetManager.hpp"

class User {
public:
    User(const InetManager& addr_info, const std::string& username = "unnamed") : _username(username), _addr_info(addr_info) {
    }

    User(InetManager&& addr_info, std::string&& username) : _username(std::move(username)), _addr_info(std::move(addr_info)) {
    }

    User(InetManager &&addr_info) : _addr_info(std::move(addr_info)) {
    }

    [[nodiscard]] const InetManager& GetUserAddr() const {
        return _addr_info;
    }

    InetManager GetUserAddr() {
        return _addr_info;
    }

    [[nodiscard]] std::string GetUsername() const {
#ifdef DEBUG_MODE
        return _addr_info.GetIp() + ':' + std::to_string(_addr_info.GetPort());
#else
        return _username;
#endif
    }

private:
    std::string _username;
    InetManager _addr_info;
};
