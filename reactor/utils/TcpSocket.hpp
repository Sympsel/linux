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
        const int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (sockfd < 0) {
            LOG_FATAL() << "socket error";
            exit(2);
        }
        constexpr int opt = 1;
        ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof opt);

        LOG_INFO() << std::format("socket success, fd: {}", sockfd);

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
        LOG_INFO() << std::format("bind success, fd: {}", _sockfd);
    }

    void listen(const int backlog = 32) const {
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

    [[nodiscard]] static int accept(const int listenSockFd, InetAddr &clientAddr) {
        sockaddr_in temp{};
        socklen_t addrLen = sizeof temp;

        const int clientSockFd = ::accept(listenSockFd, reinterpret_cast<sockaddr *>(&temp), &addrLen);
        if (clientSockFd < 0) {
            return -1;
        }
        clientAddr.setAddr(temp);
        LOG_INFO() << std::format("accept success, fd: {}", clientSockFd);
        return clientSockFd;
    }

    [[nodiscard]] bool connect(const InetAddr &peerAddr) const {
        return connect(_sockfd, peerAddr);
    }

    static bool connect(const int sockFd, const InetAddr &peerAddr) {
        if (::connect(sockFd, reinterpret_cast<const sockaddr *>(&peerAddr.getAddr()), peerAddr.getAddrLen()) < 0) {
            LOG_ERROR() << std::format("connect error, fd: {}", strerror(errno));
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

    ssize_t receive(std::string &outMsg, const size_t size = 1024) const {
        return receive(_sockfd, outMsg, size);
    }

    static ssize_t receive(const int receiveFd, std::string& outMsg, const size_t size = 1024) {
        char outBuffer[size];
        const ssize_t n = ::recv(receiveFd, outBuffer, size - 1, 0);
        if (n < 0) {
            return -1;
        }
        if (n == 0) {
            // 对端关闭连接
            return 0;
        }
        outBuffer[n] = '\0';
        outMsg.append(outBuffer, n);
        return n;
    }

    size_t sendTo(const std::string& msgToSend) const {
        return sendTo(_sockfd, msgToSend);
    }

    static ssize_t sendTo(const int destFd, const std::string& msgToSend) {
        ssize_t n = ::send(destFd, msgToSend.c_str(), msgToSend.size(), 0);
        if (n < 0) {
            LOG_ERROR() << std::format("send error: {}", strerror(errno));
            return -1;
        }
        return n;
    }

    ~TcpSocket() {
        if (_sockfd >= 0) {
            close(_sockfd);
            LOG_INFO() << std::format("close socket, fd: {}", _sockfd);
        }
    }

    static std::unique_ptr<TcpSocket> buildListenSocket(const int port) {
        auto listenSocket = std::make_unique<TcpSocket>(port);
        listenSocket->bind();
        listenSocket->listen();
        return listenSocket;
    }

private:
    int _sockfd;
    InetAddr _inetAddr;
};
