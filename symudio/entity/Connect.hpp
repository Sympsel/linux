#pragma once

#include <string>

#include "Any.hpp"
#include "Buffer.hpp"
#include "Channel.h"
#include "Socket.hpp"

class Connection;

using ConnectionPtr = std::unique_ptr<Connection>;

class Connection {
    using ConnectedCallback = std::function<void(const ConnectionPtr &)>;
    using MessageCallback = std::function<void(const ConnectionPtr &, Buffer &)>;
    using ClosedCallback = std::function<void(const ConnectionPtr &)>;
    using AnyEventCallback = std::function<void(const ConnectionPtr &)>;

    enum class Status {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
    };

public:
    Connection(Poller *poller, int fd)
        : _socket(fd)
          , _channel(poller, fd) {
    }

    ~Connection() {
    }

    void send(char *data, size_t len);

    // 稍后断连
    void shutdown();

    void enableReleaseInactive(uint32_t sec) {
        _releaseInactive = true;
    }

    void disableReleaseInactive() {
        _releaseInactive = false;
    }

    // 协议切换，重置上下文及阶段性处理函数
    void upgrade(const Any &context
                 , const ConnectedCallback &connCb
                 , const MessageCallback &msgCb
                 , const ClosedCallback &closedCb
                 , const AnyEventCallback &anyEventCb);

private:
    // 连接 ID 兼 定时器 ID
    uint64_t _id{};
    Socket _socket;
    // 是否启用非活跃自动断连
    bool _releaseInactive{};
    Channel _channel;
    Buffer _inBuffer;
    Buffer _outBuffer;
    // 请求的接收处理上下文
    Any _context;

    // Callback
    ConnectedCallback _connectedCb;
    MessageCallback _messageCb;
    ClosedCallback _closedCb;
    AnyEventCallback _anyEventCb;
};
