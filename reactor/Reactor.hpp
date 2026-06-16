#pragma once

#include <memory>
#include <unordered_map>

#include "Log.hpp"
#include "Poller.hpp"
#include "Listener.h"
#include "Common.hpp"

class Connection;

class Reactor {
private:
    bool isConnectionExists(const int sockFd) const {
        return _connections.contains(sockFd);
    }

public:
    explicit Reactor() : _epoller(std::make_unique<Poller>()) {
    }

    void addConnection(const std::shared_ptr<Connection> &connection) {
        // 把文件描述符和事件写到内核
        const int fd = connection->getSockFd();
        const auto eventItem = connection->getEventItem();
        _epoller->addEventItem(fd, eventItem);
        // 托管到容器
        _connections[fd] = connection;
        connection->_reactor = this;
    }

    void removeConnection(const std::shared_ptr<Connection>& connection) {
        if (const int fd = connection->getSockFd(); isConnectionExists(fd)) {
            _connections.erase(fd);
            _epoller->removeEventItem(fd);
        }
    }

    void updateEventItem(const int fd, const uint32_t eventItem) const {
        _epoller->updateEventItem(fd, eventItem);
    }

    void dispatcher() {
        while (true) {
            constexpr int timeout = 2000;
            int n = _epoller->waitReadyEvents(_revs, sizeof _revs, timeout);
            for (int i = 0; i < n; ++i) {
                int sockFd = _revs[i].data.fd;
                auto reventItem = _revs[i].events;

                // EPOLLHUP 对端关闭连接
                if (reventItem & ERROR || reventItem & HUP) {
                    reventItem = IN | OUT;
                }

                if (reventItem & IN) {
                    if (isConnectionExists(sockFd)) {
                        _connections[sockFd]->receiver();
                    }
                }
                if (reventItem & OUT) {
                    if (isConnectionExists(sockFd)) {
                        _connections[sockFd]->sender();
                    }
                }
            }
        }
    }

    ~Reactor() = default;

private:
    static constexpr size_t MAX_EVENTS = 128;
    // epoll模型
    std::unique_ptr<Poller> _epoller;
    // 组织所有的 Connection
    std::unordered_map<int, std::shared_ptr<Connection> > _connections;
    // 就绪队列
    epoll_event _revs[MAX_EVENTS]{};

};
