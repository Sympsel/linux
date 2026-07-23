#pragma once

#include <string>

#include "Any.hpp"
#include "Buffer.hpp"
#include "Channel.h"
#include "Socket.hpp"

class Connection;

using ConnectionPtr = std::shared_ptr<Connection>;

class Connection : public std::enable_shared_from_this<Connection> {
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

private:
    void handleRead() {
        char buf[65536];
        ssize_t ret = _socket.nonBlockRecv(buf, sizeof buf - 1);
        if (ret < 0) {
            LOG_ERROR() << "Recv Failed!";
            shutdownInLoop();
        } else if (ret > 0) {
            _inBuffer.write(buf, ret);
            if (_inBuffer.getReadableSize() > 0) {
                if (_messageCb) {
                    _messageCb(shared_from_this(), _inBuffer);
                }
            }
        } else {}
    }

    void handleWrite() {
        ssize_t ret = _socket.send(const_cast<char *>(_outBuffer.peek()), _outBuffer.getReadableSize());
        if (ret < 0) {
            LOG_ERROR() << "Send Failed!";
            if (_inBuffer.getReadableSize() > 0) {
                if (_messageCb) {
                    _messageCb(shared_from_this(), _inBuffer);
                }
            }
            releaseInLoop();
        } else {
            _outBuffer.retrieve(ret);
            if (_outBuffer.getReadableSize() == 0) {
                // 没有数据待发送
                _channel.setWriteListen(false);
                if (_status == Status::DISCONNECTING) {
                    releaseInLoop();
                }
            }
        }

    }

    void handleAnyEvent() {
        // 刷新连接活跃 & 调用组件使用者的任意事件处理
        if (_releaseInactive) {
            // todo _loop
        }
        if (_anyEventCb) {
            _anyEventCb(shared_from_this());
        }
    }

    void handleClose() {
        if (_inBuffer.getReadableSize() > 0) {
            if (_messageCb) {
                _messageCb(shared_from_this(), _inBuffer);
            }
        }
        releaseInLoop();
    }

    void handleError() {
        handleClose();
    }

    void releaseInLoop() {
        _status = Status::CONNECTED;
        _channel.remove();
        _socket.close();
        // todo 取消定时任务
        if (_closedCb) {
            _closedCb(shared_from_this());
        }
        if (_serverClosedCb) {
            _serverClosedCb(shared_from_this());
        }
    }

    // 连接建立后，进行的各种设置
    void establishInLoop() {
        if (_status == Status::CONNECTING) {

            _status = Status::CONNECTED;
            _channel.setReadListen(true);
            if (_connectedCb) {
                _connectedCb(shared_from_this());
            }
        }

    }

    void sendInLoop(char *data, size_t len);

    void shutdownInLoop();

    void enableReleaseInactiveInLoop(uint32_t sec);

    void disableReleaseInactiveInLoop();

    void upgradeInLoop(const Any &context
                       , const ConnectedCallback &connCb
                       , const MessageCallback &msgCb
                       , const ClosedCallback &closedCb
                       , const AnyEventCallback &anyEventCb);

public:
    Connection(Poller *poller, int fd)
        : _socket(fd)
          , _channel(poller, fd) {
    }

    ~Connection() {
    }

    uint64_t id() {
        return _id;
    }

    Status status() {
        return _status;
    }

    int fd() {
        return _socket.fd();
    }

    bool isConnected() {
        return _status == Status::CONNECTED;
    }

    void setContext(const Any &context);

    Any &context();

    void send(char *data, size_t len);

    // 稍后断连
    void shutdown();

    void enableReleaseInactive(uint32_t sec) {
        _releaseInactive = true;
        //
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

    void setConnectedCallback(const ConnectedCallback &cb);

    void setMessageCallback(const MessageCallback &cb);

    void setClosedCallback(const ClosedCallback &cb);

    void setAnyEventCallback(const AnyEventCallback &cb);

private:
    // 连接 ID 兼 定时器 ID
    uint64_t _id{};
    Socket _socket;
    Status _status;
    // 是否启用非活跃自动断连
    bool _releaseInactive{};
    Channel _channel;
    Buffer _inBuffer;
    Buffer _outBuffer;
    // 请求的接收处理上下文
    Any _context;

    // 用户Callback
    ConnectedCallback _connectedCb;
    MessageCallback _messageCb;
    ClosedCallback _closedCb;
    AnyEventCallback _anyEventCb;
    // 组件内的连接关闭回调
    ClosedCallback _serverClosedCb;
};
