#pragma once

#include <memory>

#include "InetAddr.hpp"

class Reactor;

class Connection : public std::enable_shared_from_this<Connection> {
public:
    explicit Connection() : _eventItem(), _reactor(nullptr) {
    }

    virtual void receiver() = 0;

    virtual void sender() = 0;

    virtual void expecter() = 0;

    virtual int getSockFd() = 0;

    [[nodiscard]] uint32_t getEventItem() const {
        return _eventItem;
    }

    void setEventItem(const uint32_t eventItem) {
        _eventItem = eventItem;
    }

    void setInetAddr(const InetAddr& clientAddr) {
        _clientAddr = clientAddr;
    }

    virtual ~Connection() = default;

protected:
    InetAddr _clientAddr;
    uint32_t _eventItem;

public:
    Reactor *_reactor;
};
