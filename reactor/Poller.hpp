#pragma once

#include <sys/epoll.h>
#include <cstring>

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
            LOG_FATAL() << std::format("epollFd 创建失败, fd={}", _epFd);
            exit(ERR::OF_EPOLL);
        }
        LOG_INFO() << std::format("epollFd 创建成功, fd={}", _epFd);
    }

    ~Poller() {
        if (_epFd >= 0) {
            ::close(_epFd);
            _epFd = -1;
        }
    }

    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;
    Poller(Poller&&) = delete;
    Poller& operator=(Poller&&) = delete;

    void addEventItem(const int fd, const uint32_t eventItem) const {
        epoll_event epEvent{};
        epEvent.data.fd = fd;
        epEvent.events = eventItem;
        if (::epoll_ctl(_epFd, EPOLL_CTL_ADD, fd, &epEvent) < 0) {
            LOG_ERROR() << std::format("epoll_ctl ADD 失败, fd={}, error={}", fd, strerror(errno));
        } else {
            LOG_DEBUG() << std::format("epoll_ctl ADD 成功, fd={}, events={}", fd, eventItem);
        }
    }

    void updateEventItem(const int fd, const uint32_t eventItem) const {
        epoll_event epEvent{};
        epEvent.data.fd = fd;
        epEvent.events = eventItem;

        if (::epoll_ctl(_epFd, EPOLL_CTL_MOD, fd, &epEvent) < 0) {
            LOG_ERROR() << std::format("epoll_ctl MOD 失败, fd={}, error={}", fd, strerror(errno));
        } else {
            LOG_DEBUG() << std::format("epoll_ctl MOD 成功, fd={}, events={}", fd, eventItem);
        }
    }

    void removeEventItem(const int fd) const {
        if (::epoll_ctl(_epFd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
            LOG_ERROR() << std::format("epoll_ctl DEL 失败, fd={}, error={}", fd, strerror(errno));
        } else {
            LOG_DEBUG() << std::format("epoll_ctl DEL 成功, fd={}", fd);
        }
    }

    int waitReadyEvents(epoll_event* revs, const int num, const int timeout) const {
        const int n = epoll_wait(_epFd, revs, num, timeout);
        if (n < 0) {
            if (errno == EINTR) {
                LOG_DEBUG() << "epoll_wait 被信号中断";
                return 0;
            }
            LOG_ERROR() << std::format("epoll_wait 失败, error={}", strerror(errno));
        }
        return n;
    }

    [[nodiscard]] int getEpollFd() const {
        return _epFd;
    }

private:
    int _epFd{-1};
};
