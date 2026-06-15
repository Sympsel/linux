#pragma once
#include "Connection.hpp"
#include "TcpSocket.hpp"

class Listener : public Connection {
public:
    Listener(const uint16_t port)
    : _port(port)
    , _listenSocket(std::make_unique<TcpSocket>(_port)) {
        _listenSocket->bind();
        _listenSocket->listen();
        LOG_INFO() << std::format("PollServer: 监听套接字创建成功, 端口: {}", port);
}


    void receiver() override {

    }

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
