#pragma once

#include <memory>
#include <unordered_map>

#include "Listener.h"
#include "Poller.hpp"
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

    void addListener(const std::shared_ptr<Listener>& listener) {
        listener->setReactor(this);
        const int fd = listener->getListenFd();
        _epoller->addEventItem(fd, true, false);
        _listener = listener;
    }

    void removeConnection(const std::shared_ptr<Connection> &connection) {
        if (const int fd = connection->getSockFd(); isConnectionExists(fd)) {
            _connections.erase(fd);
            _epoller->removeEventItem(fd);
        }
    }

    [[deprecated]] void updateEventItem(const int fd, const uint32_t eventItem) const {
        _epoller->updateEventItem(fd, eventItem);
    }

    void setReadWriteEventItem(const int fd, const bool careReadable, const bool careWritable) const {
        updateReadWriteEventItem(fd, careReadable, careWritable);
    }

    void updateReadWriteEventItem(const int fd, const bool careReadable, const bool careWritable) const {
        if (isConnectionExists(fd)) {
            uint32_t eventItem;
            Poller::setEventItemReadWrite(eventItem, careReadable, careWritable);
            _epoller->updateEventItem(fd, eventItem);
        }
    }

    void dispatcher() {
        while (true) {
            constexpr int timeout = 2000;
            const int n = _epoller->waitReadyEvents(_revs, sizeof _revs, timeout);
            for (int i = 0; i < n; ++i) {
                int sockFd = _revs[i].data.fd;
                auto reventItem = _revs[i].events;

                // EPOLLHUP 对端关闭连接
                if (reventItem & ERROR || reventItem & HUP) {
                    Poller::setEventItemReadWrite(reventItem, true, true);
                }

                if (_listener && sockFd == _listener->getListenFd()) {
                    _listener->accept();
                    continue;
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
    // 监听器
    std::shared_ptr<Listener> _listener;
    // 连接管理
    std::unordered_map<int, std::shared_ptr<Connection> > _connections;
    // 就绪队列
    epoll_event _revs[MAX_EVENTS]{};
};
