#pragma once

#include "Common.hpp"
#include "InetAddr.hpp"
#include "Buffer.hpp"

class Reactor;

class Connection {
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

    void setReadWriteEventItem(const bool readable, const bool writable) {
        _eventItem = ET;
        if (readable) {
            _eventItem |= IN;
        }
        if (writable) {
            _eventItem |= OUT;
        }
    }

    void setInetAddr(const InetAddr& clientAddr) {
        _clientAddr = clientAddr;
    }

    virtual ~Connection() = default;

protected:
    InetAddr _clientAddr;
    uint32_t _eventItem;

    Buffer _inBuffer;
    Buffer _outBuffer;
public:
    Reactor *_reactor;
};
