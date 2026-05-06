#pragma once

#include <vector>

#include "User.hpp"
#include "ThreadPool.hpp"

/**
 * @brief handle the message distribute
 */
class RouteServer {
private:
    bool IsOnline(const InetManager &addr) {
        LockGuard lockguard(_lock);
        for (auto &user: _online_list) {
            if (user.GetUserAddr() == addr) {
                return true;
            }
        }
        return false;
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

    void RouteMessage(const std::string &message, const User &who, const int sockfd) {
        if (message.empty()) {
            return;
        }
        if (!IsOnline(who.GetUserAddr())) {
            LockGuard lockguard(_lock);
            AddUser(who);
        }

        LockGuard lockguard(_lock);
        const std::string &formated_msg = message;
        for (auto &user: _online_list) {
            user.GetUserAddr().SendTo(sockfd, formated_msg);
        }
    }

    ~RouteServer() = default;

private:
    std::vector<User> _online_list;
    Mutex _lock;
};
