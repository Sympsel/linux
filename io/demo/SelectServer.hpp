#pragma once

#include <sys/select.h>
#include <print>

#include "../TcpServer.hpp"

class SelectServer {
private:
    static constexpr int _size = sizeof(fd_set) * 8;

    // 连接管理器
    void accepter() {
        InetAddr clientAddr;
        int fd = TcpSocket::accept(_listenSocket->getSockfd(), clientAddr);
        std::println("获取一个新连接，fd: {}", fd);
        if (fd >= 0) {
            const auto pos = std::find(_arrayFds, _arrayFds + _size, -1);
            if (pos == _arrayFds + _size) {
                std::println("连接已满，请稍候再试");
                close(fd);
                *pos = fd;
            } else {
                close(fd);
                *pos = fd;
            }
        }
    }

    void ioHandler(int &fd) {
        std::println("处理客户端数据，文件描述符为 {}", fd);
        char buffer[1024];
        ssize_t n = recv(fd, buffer, sizeof buffer - 1, 0);
        if (n == 0) {
            std::println("连接断开，文件描述符为 {}", fd);
            close(fd);
            fd = -1;
        } else if (n < 0) {
            std::println("读取数据失败，文件描述符为 {}", fd);
        }
    }

public:
    explicit SelectServer(const uint16_t port)
        : _port(port), _listenSocket(std::make_unique<TcpSocket>(port)) {
        _listenSocket->bind();
        if (_listenSocket->listen()) {
            std::println("SelectServer: 监听套接字创建成功");
        }
        for (int &arrayFd: _arrayFds) {
            arrayFd = -1;
        }
        // 把监听套接字添加到辅助数组
        _arrayFds[0] = _listenSocket->getSockfd();
    }

    void dispatcher() {
        while (true) {
            fd_set rfds;
            FD_ZERO(&rfds);
            int maxFd = -1;
            for (int &fd: _arrayFds) {
                if (fd == -1) {
                    continue;
                }
                // 把所有有效套接字添加到监听集合
                FD_SET(fd, &rfds);
                maxFd = std::max(maxFd, fd);
            }
            timeval timeout = {
                2, 0
            };
            // 监听所关注的文件描述符是否“就绪”，即是否有数据可读
            int n = ::select(maxFd + 1, &rfds, nullptr, nullptr, &timeout);
            if (n < 0) {
                std::println("SelectServer: select失败");
                continue;
            }
            std::println("SelectServer: 接收到新连接， 数量为 {}", n);
            for (auto &fd: _arrayFds) {
                if (fd == -1) {
                    continue;
                }
                if (FD_ISSET(fd, &rfds)) {
                    if (fd == _listenSocket->getSockfd()) {
                        // 如果是监听套接字，则将新连接添加到辅助数组
                        accepter();
                    } else {
                        ioHandler(fd);
                    }
                }
            }
            accepter();
        }
    }

    ~SelectServer() = default;

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listenSocket;
    int _arrayFds[_size];
};
