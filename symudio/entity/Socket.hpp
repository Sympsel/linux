#pragma once

#include <cstdint>
#include <string>
#include <unistd.h>
#include <sys/socket.h>

// class INetAddr {
//
// private:
//     std::string _ip;
//     uint16_t _port;
//
// };

class Socket {
public:
    Socket() : _sockFd(-1) {
    }

    ~Socket(int sockfd) : _sockFd(sockfd) {
        close();
    }

    bool create() {
        // _sockFd = socket();
    }

    bool bind(const std::string &ip, uint16_t port) {
    }

    bool listen(const int backlog = 32) {
    }

    bool connect(const std::string &ip, uint16_t port) {
    }

    int accept() {
    }

    ssize_t recv(void *buf, size_t len, int flag = 0) {
    }

    ssize_t send(void *buf, size_t len, int flag = 0) {
    }

    void close() {
    }

    bool createServer(uint16_t port, const std::string &ip = "0.0.0.0") {
    }

    bool createClient(uint16_t port, const std::string &ip) {
    }

    // 开启地址端口重用
    void reuseAddress() {
    }

    void nonblock() {
    }

private:
    int _sockFd;
};
