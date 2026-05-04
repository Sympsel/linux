#pragma once

#include <iostream>
#include <vector>

#include "User.hpp"

class Route {
private:
    bool IsOnline(const User& who) {
        LockGuard lockguard(_lock);
        for (auto& user : _online_list) {
            if (user.GetUserAddr() == who.GetUserAddr()) {
                return true;
            }
        }
        return false;
    }

    void AddUser(const User& who) {
        _online_list.emplace_back(who);
    }

    Route() = default;
public:
    Route(const Route&) = delete;
    Route(Route&&) = delete;
    Route& operator=(const Route&) = delete;
    Route& operator=(Route&&) = delete;

    static Route& GetInstance() {
        static Route instance;
        return instance;
    }

    void RouteMessage(const std::string& message, const User& who, int sockfd) {
        LockGuard lockguard(_lock);
        if (!IsOnline(who)) {
            LockGuard lockguard(_lock);
            AddUser(who);
        }

        for (auto& user : _online_list) {
            user.GetUserAddr().SendTo(sockfd, message);
        }
    }

    ~Route() = default;

private:
    std::vector<User> _online_list;
    Mutex _lock;
};
