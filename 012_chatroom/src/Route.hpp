#pragma once

#include <iostream>
#include <vector>

#include "User.hpp"
#include "ThreadPool.hpp"

class Route {
private:
    bool IsOnline(const InetManager& addr) {
        LockGuard lockguard(_lock);
        for (auto& user : _online_list) {
            if (user.GetUserAddr() == addr) {
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

    void RouteMessage(const std::string& message, const User& who, const int sockfd) {
        if (!IsOnline(who.GetUserAddr())) {
            LockGuard lockguard(_lock);
            AddUser(who);
        }

        LockGuard lockguard(_lock);
        std::string formated_msg = who.GetUsername();
        formated_msg += ": ";
        formated_msg += message;
        for (auto& user : _online_list) {
            user.GetUserAddr().SendTo(sockfd, formated_msg);
        }
    }

    ~Route() = default;

private:
    std::vector<User> _online_list;
    Mutex _lock;
};
