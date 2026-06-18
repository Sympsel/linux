#pragma once

#include <fcntl.h>
#include <functional>
#include <memory>

#include "../core/Connection.hpp"
#include "TcpSocket.hpp"

/**
 * @brief 采用工厂模式
 */
class Listener {
private:
    static void setNonBlock(int fd) {
        const int flags = ::fcntl(fd, F_GETFL);
        if (flags < 0) {
            LOG_ERROR() << std::format("设置文件描述符 {} 非阻塞失败", fd);
        }
        ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

public:
    using ConnectionFactory = std::function<std::shared_ptr<Connection>(int)>;

    explicit Listener(const uint16_t port, ConnectionFactory factory)
        : _port(port)
          , _listenSocket(TcpSocket::buildListenSocket(_port))
          , _connectionFactory(std::move(factory))
          , _reactor(nullptr) {
        setNonBlock(_listenSocket->getSockfd());
        LOG_INFO() << std::format("监听套接字创建成功, 端口: {}", port);
    }


    void accept() const;

    void setReactor(Reactor* reactor) {
        _reactor = reactor;
    }

    ~Listener() = default;

    [[nodiscard]] int getListenFd() const {
        return _listenSocket->getSockfd();
    }

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listenSocket;
    ConnectionFactory _connectionFactory;
    Reactor* _reactor;
};
