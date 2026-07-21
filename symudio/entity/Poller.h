#pragma once

#include <cstring>
#include <unordered_map>
#include <sys/epoll.h>

#include "Channel.h"
#include "Log.hpp"

#define MAX_EPOLL_EVENT 1024

/**
 *
 * 轮询器 - 事件驱动核心
 */
class Poller {
private:
    bool update(Channel *channel, int op) {
        epoll_event ev{};
        ev.data.fd = channel->fd();
        ev.events = channel->events();
        int ret = epoll_ctl(_epFd, op, channel->fd(), &ev);
        if (ret < 0) {
            LOG_ERROR() << "Epoll_Ctrl Failed!";
            return false;
        }
        return true;
    }

    bool hasChannel(Channel *channel) {
        return _channels.contains(channel->fd());
    }

public:
    Poller() {
        _epFd = epoll_create1(0);
        if (_epFd < 0) {
            LOG_FATAL() << "Epoll Create Failed!";
            exit(EXIT_FAILURE);
        }
    }

    bool updateEvent(Channel *channel) {
        return hasChannel(channel)
                   ? update(channel, EPOLL_CTL_MOD)
                   : update(channel, EPOLL_CTL_ADD);
    }

    void removeEvent(Channel *channel) {
        update(channel, EPOLL_CTL_DEL);
        _channels.erase(channel->fd());
    }

    // 开始监控，返回活跃连接
    void poll(std::vector<Channel *>& actives);

private:
    int _epFd;
    epoll_event _epEvents[MAX_EPOLL_EVENT];
    std::unordered_map<int, Channel *> _channels;
};

