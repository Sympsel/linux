#pragma once

#include <fcntl.h>

#include "Connection.hpp"
#include "TcpSocket.hpp"

class Listener : public Connection {
private:
    static void setNonBlock(int fd) {
        const int flags = ::fcntl(fd, F_GETFL);
        if (flags < 0) {
            LOG_ERROR() << std::format("设置文件描述符 {} 非阻塞失败", fd);
        }
        ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

public:
    explicit Listener(const uint16_t port)
        : _port(port)
          , _listenSocket(TcpSocket::buildListenSocket(_port)) {
        setNonBlock(_listenSocket->getSockfd());
        LOG_INFO() << std::format("PollServer: 监听套接字创建成功, 端口: {}", port);
    }

    void receiver() override;

    void sender() override {
    }

    void expecter() override {
    }

    ~Listener() override = default;

    int getSockFd() override {
        return _listenSocket->getSockfd();
    }

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listenSocket;
};
