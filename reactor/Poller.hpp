#pragma once

#include <sys/epoll.h>
#include "Log.hpp"

#include "Common.hpp"

/**
 * @brief 基于 epoll 的，监听所有文件描述符是否就绪
 */
class Poller {
public:
    explicit Poller() {
        _epFd = ::epoll_create1(EPOLL_CLOEXEC);
        if (_epFd < 0) {
            LOG_FATAL() << std::format("epollFd 创建失败");
            exit(ERR::OF_EPOLL);
        }
        LOG_INFO() << std::format("epollFd 创建成功");
    }

    ~Poller() {
    }

    bool create() {
    }

    bool destroy() {
    }

    bool getEvents() {
    }

    bool setFdEvent() {
    }

    void addEventItem(const int fd, const uint32_t eventItem) const {
        epoll_event epEvent{};
        epEvent.data.fd = fd;
        epEvent.events = eventItem;
        ::epoll_ctl(_epFd, EPOLL_CTL_ADD, fd, &epEvent);
    }

    int waitReadyEvents(epoll_event* revs, const int num, const int timeout) const {
        int n = epoll_wait(_epFd, revs, num, timeout);
        return n;
    }

private:
    int _epFd;
};
