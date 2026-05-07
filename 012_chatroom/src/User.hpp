#pragma once

#include <utility>

// #define DEBUG_MODE

#include "UdpSocket.hpp"
#include "Config.hpp"

class User {
public:
    User() {
        InitConf();
    }

    // ReSharper disable once CppNonExplicitConvertingConstructor
    User(InetAddr addr_info, std::string username) : _username(std::move(username)),
                                                     _addr_info(std::move(addr_info)) {
        if (_username.empty()) {
            _username = Conf::default_username;
        }
        InitConf();
    }

    explicit User(InetAddr addr_info) : _addr_info(std::move(addr_info)) {
        InitConf();
    }

    [[nodiscard]] const InetAddr &GetUserInetAddr() const {
        return _addr_info;
    }

    [[nodiscard]] std::string GetUsername() const {
#ifdef DEBUG_MODE
        return _addr_info.GetIp() + ':' + std::to_string(_addr_info.GetPort());
#else
        return _username;
#endif
    }

    void SetUsername(const std::string &username) {
        _username = username;
    }

    void SetInetAddr(const InetAddr &addr_info) {
        _addr_info = addr_info;
    }

    ~User() = default;

private:
    std::string _username;
    InetAddr _addr_info;
};
