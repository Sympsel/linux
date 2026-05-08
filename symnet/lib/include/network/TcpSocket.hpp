#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>

#include "SocketUtils.hpp"

class TcpSocket {
private:
    static int CreateSockfd() {
        const int sockfd = SocketUtils::CreateSocket(AF_INET,  SOCK_STREAM, 0);
        if (sockfd < 0) {
            LOG_FATAL() << "socket error";
            exit(2);
        }
        LOG_INFO() << "socket success. [fd: " << sockfd << "]";
        return sockfd;
    }
public:
    static bool Listen(const int sockfd, const int backlog = 32) {
        if (const int ret = listen(sockfd, backlog); ret < 0) {
            LOG_ERROR() << "listen error";
            return false;
        }
        return true;
    }

    /**
     *
     * @param sockfd server_sockfd
     * @param client_addr output client_addr
     * @return  -1: error
     */
    [[nodiscard]] static int Accept(const int sockfd, InetAddr &client_addr) {
        sockaddr_in temp{};
        socklen_t addrlen = sizeof temp;

        const int conn_sockfd = accept(sockfd, reinterpret_cast<sockaddr *>(&temp), &addrlen);
        if (conn_sockfd < 0) {
            LOG_WARN() << "accept error";
            return -1;
        }
        client_addr.SetAddr(temp);
        LOG_INFO() << "accept success. [fd: " << conn_sockfd << "]";
        return conn_sockfd;
    }

    static bool Connect(const int sockfd, const InetAddr &peer) {
        if (connect(sockfd, reinterpret_cast<const sockaddr *>(&peer.GetAddr()), peer.GetAddrLen()) < 0) {
            LOG_ERROR() << "connect error: " << strerror(errno);
            return false;
        }
        return true;
    }

    TcpSocket() {
        _sockfd = CreateSockfd();
    }

    [[nodiscard]] const int &GetSockfd() const {
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
};
