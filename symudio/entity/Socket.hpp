#pragma once

#include <cstdint>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "Log.hpp"


class Socket {
public:
    Socket(int sockfd = -1) : _sockFd(sockfd) {
    }

    ~Socket() {
        close();
    }

    int fd() {
        return _sockFd;
    }

    bool create() {
        _sockFd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (_sockFd < 0) {
            LOG_ERROR() << "Create Socket Failed!";
            return false;
        }

        return true;
    }

    bool bind(const std::string &ip, uint16_t port) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(port);
        addr.sin_addr.s_addr = ::inet_addr(ip.c_str());
        socklen_t len = sizeof addr;
        int ret = ::bind(_sockFd, reinterpret_cast<sockaddr *>(&addr), len);
        if (ret < 0) {
            LOG_ERROR() << "Bind Address Failed!";
            return false;
        }
        return true;
    }

    bool listen(const int backlog = 32) {
        int ret = ::listen(_sockFd, backlog);
        if (ret < 0) {
            LOG_ERROR() << "Listen Failed!";
            return false;
        }
        return true;
    }

    bool connect(const std::string &ip, uint16_t port) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(port);
        addr.sin_addr.s_addr = ::inet_addr(ip.c_str());
        socklen_t len = sizeof addr;
        int ret = ::connect(_sockFd, reinterpret_cast<sockaddr *>(&addr), len);
        if (ret < 0) {
            LOG_ERROR() << "Connect Server Failed!";
            return false;
        }
        return true;
    }

    int accept() {
        int ret = ::accept(_sockFd, nullptr, nullptr);
        if (ret < 0) {
            LOG_ERROR() << "Accept Failed!";
        }
        return ret;
    }

    ssize_t recv(void *buf, size_t len = 1024, int flag = 0) {
        ssize_t ret = ::recv(_sockFd, buf, len, flag);
        if (ret <= 0) {
            if (errno == EAGAIN || errno == EINTR) {
                return 0;
            }
            LOG_ERROR() << "Recv Failed!";
            return -1;
        }
        return ret;
    }

    ssize_t nonBlockRecv(void *buf, size_t len) {
        return recv(buf, len, MSG_DONTWAIT);
    }

    ssize_t send(void *buf, size_t len, int flag = 0) {
        ssize_t ret = ::send(_sockFd, buf, len, flag);
        if (ret < 0) {
            LOG_ERROR() << "Send Failed!";
            return -1;
        }
        return ret;
    }

    ssize_t nonBlockSend(void *buf, size_t len) {
        return send(buf, len, MSG_DONTWAIT);
    }

    void close() {
        if (_sockFd != -1) {
            ::close(_sockFd);
            _sockFd = -1;
        }
    }

    bool createServer(uint16_t port, const std::string &ip = "0.0.0.0", bool isBlock = false) {
        if (!create()) {
            return false;
        }
        if (isBlock) {
            nonBlock();
        }
        if (!bind(ip, port)) {
            return false;
        }
        if (!listen()) {
            return false;
        }
        reuseAddress();
        return true;
    }

    bool createClient(uint16_t port, const std::string &ip) {
        if (!create()) {
            return false;
        }
        if (!connect(ip, port)) {
            return false;
        }
        return true;
    }

    // 开启地址端口重用
    void reuseAddress() {
        int opt = 1;
        // SOL_SOCKET 表示通用套接字选项
        setsockopt(_sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
        opt = 1;
        setsockopt(_sockFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int));
    }

    void nonBlock() {
        int flag = ::fcntl(_sockFd, F_GETFL);
        ::fcntl(_sockFd, flag | O_NONBLOCK);
    }

private:
    int _sockFd;
};
