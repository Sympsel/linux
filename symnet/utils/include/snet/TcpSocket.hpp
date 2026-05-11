#pragma once

#include <cstring>

#include "Socket.hpp"


class TcpSocket : public ITcpSocket {
public:
    explicit TcpSocket(const int sockfd = -1) {
        _sockfd = sockfd;
    }

    void CreateSocket() override {
        if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            exit(CREATE_ERROR);
        }
    }

    void BindSocket(const in_port_t port) override {
        sockaddr_in _addr{};
        _addr.sin_addr.s_addr = htonl(INADDR_ANY);
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        if (const int ret = bind(_sockfd, reinterpret_cast<sockaddr *>(&_addr), sizeof _addr); ret == -1) {
            exit(BIND_ERROR);
        }
    }

    void ListenSocket() override {
        if (const int ret = listen(_sockfd, default_a); ret == -1) {
            exit(LISTEN_ERROR);
        }
        listen(_sockfd, default_a);
    }

    std::shared_ptr<ITcpSocket> Acceptor(InetAddr &client_addr) override {
        sockaddr_in peer{};
        socklen_t len = sizeof peer;
        int sockfd = accept(_sockfd, reinterpret_cast<sockaddr *>(&peer), &len);
        if (sockfd < 0) {
            return nullptr;
        }
        client_addr = peer;
        return std::make_shared<TcpSocket>(sockfd);
    }

    ssize_t Recv(std::string &str_to_fill) override {
        // todo
        char buffer[1024];
        const ssize_t n = recv(_sockfd, buffer, sizeof buffer - 1, 0);
        if (n < 0) {
            return n;
        }
        buffer[n] = '\0';
        str_to_fill += buffer;
        return n;
    }

    bool Send(const std::string &str) override {
        return send(_sockfd, str.c_str(), str.size(), 0);
    }

    const int &GetSockfd() override {
        return _sockfd;
    }

    bool Connect(const InetAddr& peer) override {
        if (const int ret = connect(_sockfd, reinterpret_cast<const sockaddr *>(&peer.GetAddr()), peer.GetAddrLen()); ret < 0) {
            LOG_ERROR() << "connect error: " << strerror(errno);
            return false;
        } else {
            LOG_INFO() << "connect to [" << peer.GetIp() << ":" << peer.GetPort() << "] success";
            return true;
        }
    }

    void Close() override {
        if (_sockfd >= 0) {
            close(_sockfd);
            _sockfd = -1;
        }
    }

    ~TcpSocket() override = default;
};