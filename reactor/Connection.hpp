#pragma once

#include <string>

#include "InetAddr.hpp"

class Connection {
public:
    explicit Connection() : _eventItem() {
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

    virtual ~Connection() = default;

protected:
    std::string _inBuffer;
    std::string _outBuffer;
    InetAddr _clientAddr;

    uint32_t _eventItem;
};
