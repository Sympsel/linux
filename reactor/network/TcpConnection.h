#pragma once

#include <functional>

#include "Buffer.hpp"
#include "Connection.hpp"

class Reactor;

using MessageHandler = std::function<void(Buffer &, Buffer &)>;

/**
 * @brief 用于处理原始字节流的 TCP 连接
 */
class TcpConnection : public Connection {
private:
    static std::string handleRequest(Buffer &request);

public:
    explicit TcpConnection(const int fd)
        : _sockFd(fd), _messageHandler(nullptr) {
    }

    void receiver() override;

    void sender() override;

    void expecter() override;

    void handleClose();

    int getSockFd() override {
        return _sockFd;
    }

    void setMessageHandler(MessageHandler handler) {
        _messageHandler = std::move(handler);
    }

    ~TcpConnection() override = default;

private:
    int _sockFd;
    Buffer _inBuffer;
    Buffer _outBuffer;
    MessageHandler _messageHandler;
};
