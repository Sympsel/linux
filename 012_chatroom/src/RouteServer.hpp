#pragma once

#include <vector>
#include <algorithm>

#include "User.hpp"
#include "ThreadPool.hpp"

/**
 * @brief handle the message distribute
 */
class RouteServer {
private:
    bool IsOnline(const InetAddr &addr) {
        LockGuard lockguard(_lock);
        return std::any_of(_online_list.begin(), _online_list.end(), [&addr] (const User& user) {
            return user.GetUserInetAddr() == addr;
        });
    }

    void AddUser(const User &who) {
        _online_list.emplace_back(who);
    }

    RouteServer() = default;

public:
    RouteServer(const RouteServer &) = delete;

    RouteServer(RouteServer &&) = delete;

    RouteServer &operator=(const RouteServer &) = delete;

    RouteServer &operator=(RouteServer &&) = delete;

    static RouteServer &GetInstance() {
        static RouteServer instance;
        return instance;
    }

    void RouteMessage(const std::string &message, const User &who, const UdpSocket &server_socket) {
        if (message.empty()) {
            return;
        }
        if (!IsOnline(who.GetUserInetAddr())) {
            AddUser(who);
        }

        LockGuard lockguard(_lock);
        const std::string &formated_msg = message;
        for (auto &user: _online_list) {
            server_socket.SendToAddr(formated_msg, user.GetUserInetAddr());
        }
    }

    ~RouteServer() = default;

private:
    std::vector<User> _online_list;
    Mutex _lock;
};
