#pragma once

#include "InetManager.hpp"

class User {
public:
    User(const InetManager &addr_info) : _addr_info(addr_info) {
    }

    User(InetManager &&addr_info) : _addr_info(addr_info) {
    }

    const InetManager& GetUserAddr() const {
        return _addr_info;
    }

    InetManager& GetUserAddr() {
        return _addr_info;
    }

private:
    InetManager _addr_info;
};
