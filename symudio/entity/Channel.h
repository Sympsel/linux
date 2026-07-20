#pragma once

#include <cstdint>
#include <functional>
#include <sys/epoll.h>

class Poller;

class Channel {
private:
    using EventCallback = std::function<void()>;

public:
    explicit Channel(Poller* poller, int fd) : _fd(fd), _poller(poller), _events(), _revents() {
    }

    int fd() {
        return _fd;
    }

    void setREvent(uint32_t events) {
        _revents = events;
    }

    uint32_t events() {
        return _events;
    }

    void setReadableCb(const EventCallback &cb) {
        _readCb = cb;
    }

    void setWriteableCb(const EventCallback &cb) {
        _writeCb = cb;
    }

    void setCloseableCb(const EventCallback &cb) {
        _closeCb = cb;
    }

    void setAnyEventCb(const EventCallback &cb) {
        _anyEventCb = cb;
    }


    bool isReadListening() {
        return _events & EPOLLIN;
    }

    bool isWriteListening() {
        return _events & EPOLLOUT;
    }

    void setReadListen(bool enabled) {
        if (enabled) {
            _events |= EPOLLIN;
        } else {
            _events &= ~EPOLLIN;
        }
        update();
    }

    void setWriteListen(bool enabled) {
        if (enabled) {
            _events |= EPOLLOUT;
        } else {
            _events &= ~EPOLLOUT;
        }
        update();
    }

    void disableAllListen() {
        _events = 0;
        update();
    }

    void remove();

    bool update();

    void handleEvent();

private:
    int _fd;
    Poller *_poller;
    uint32_t _events;
    uint32_t _revents;
    EventCallback _readCb;
    EventCallback _writeCb;
    EventCallback _errorCb;
    EventCallback _closeCb;
    // 任意事件触发回调
    EventCallback _anyEventCb;
};
