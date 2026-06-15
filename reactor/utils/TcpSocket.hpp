#pragma once

#include <cstring>
#include <utility>
#include <netinet/in.h>
#include <sys/socket.h>

#include "InetAddr.hpp"
#include "Log.hpp"

class TcpSocket {
private:
    static int createSockfd() {
        const int sockfd = socket(AF_INET, SOCK_STREAM, 0); // IPPROTO_TCP
        if (sockfd < 0) {
            LOG_FATAL() << "socket error";
            exit(2);
        }
        constexpr int opt = 1;
        ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof opt);

        LOG_INFO() << "socket success. [fd: " << sockfd << "]";

        return sockfd;
    }

public:
    /**
     * @warning you should initialize it before use
     */
    TcpSocket() {
        _sockfd = createSockfd();
    }

    /**
     * @param port port
     * @param ip default: 0.0.0.0 to bind any client ip
     */
    explicit TcpSocket(const in_port_t &port, const std::string &ip = "0.0.0.0") : _sockfd(createSockfd()),
        _inetAddr{port, ip} {
    }

    explicit TcpSocket(const sockaddr_in &addr) : _sockfd(createSockfd()) {
        _inetAddr.setAddr(addr);
    }

    explicit TcpSocket(InetAddr inet_addr) : _sockfd(createSockfd()), _inetAddr(std::move(inet_addr)) {
    }


    void bind() const {
        if (const int ret = ::bind(_sockfd, reinterpret_cast<const sockaddr *>(&_inetAddr.getAddr()),
                                   _inetAddr.getAddrLen()); ret < 0) {
            LOG_FATAL() << "bind error";
            exit(1);
        }
        LOG_INFO() << "bind success. [fd: " << _sockfd << "]";
    }

    void listen(const int backlog = 32)  const {
        if (const int ret = ::listen(_sockfd, backlog); ret < 0) {
            LOG_FATAL() << "listen error";
        }
    }

    /**
     * @warning this will overwrite _addr_helper with client address.
     *          if you need to preserve local address, use static Accept(sockfd, client_addr) instead.
     */
    int accept() {
        return accept(_sockfd, _inetAddr);
    }

    [[nodiscard]] static int accept(const int sockfd, InetAddr &client_addr) {
        sockaddr_in temp{};
        socklen_t addrlen = sizeof temp;

        const int conn_sockfd = ::accept(sockfd, reinterpret_cast<sockaddr *>(&temp), &addrlen);
        if (conn_sockfd < 0) {
            LOG_WARN() << "accept error";
            return -1;
        }
        client_addr.setAddr(temp);
        LOG_INFO() << "accept success. [fd: " << conn_sockfd << "]";
        return conn_sockfd;
    }

    [[nodiscard]] bool connect(const InetAddr &peer) const {
        return connect(_sockfd, peer);
    }

    static bool connect(const int sockfd, const InetAddr &peer) {
        if (::connect(sockfd, reinterpret_cast<const sockaddr *>(&peer.getAddr()), peer.getAddrLen()) < 0) {
            LOG_ERROR() << "connect error: " << strerror(errno);
            return false;
        }
        return true;
    }

    [[nodiscard]] const InetAddr &getAddr() const {
        return _inetAddr;
    }

    [[nodiscard]] const int &getSockfd() const {
        return _sockfd;
    }

    ~TcpSocket() {
        if (_sockfd >= 0) {
            close(_sockfd);
            LOG_INFO() << "close socket. [fd: " << _sockfd << "]";
        }
    }

private:
    int _sockfd;
    InetAddr _inetAddr;
};
