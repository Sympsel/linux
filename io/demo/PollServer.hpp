#pragma once

#include <poll.h>
#include <print>

#include "../utils/TcpServer.hpp"

class PollServer {
private:
    static constexpr int _size = sizeof(fd_set) * 8;

    // 连接管理器
    void accepter() {
        InetAddr clientAddr;
        int fd = TcpSocket::accept(_listenSocket->getSockfd(), clientAddr);
        std::println("获取一个新连接，fd: {}", fd);
        if (fd >= 0) {
            const auto pos = std::find_if(_fdEvents, _fdEvents + _size, [](const pollfd& fdEvent) {
                return fdEvent.fd == -1;
            });
            if (pos == _fdEvents + _size) {
                std::println("连接已满，请稍候再试");
                close(fd);
            } else {
                pos->fd = fd;
                pos->events = POLLIN;
                pos->revents = 0;
            }
        }
    }

    void ioHandler(int fd) {
        std::println("处理客户端数据，文件描述符为 {}", fd);
        char buffer[1024];
        ssize_t n = recv(fd, buffer, sizeof buffer - 1, 0);
        if (n == 0) {
            std::println("连接断开，文件描述符为 {}", fd);
            close(fd);
            for (auto &[fdItem, events, revents] : _fdEvents) {
                if (fdItem == fd) {
                    fdItem = -1;
                    revents = events = 0;
                }
            }
        } else if (n < 0) {
            std::println("读取数据失败，文件描述符为 {}", fd);
        } else {
            buffer[n] = '\0';
            std::println("收到数据: {}", buffer);
        }
    }

public:
    explicit PollServer(const uint16_t port)
        : _port(port), _listenSocket(std::make_unique<TcpSocket>(port)) {
        _listenSocket->bind();
        if (_listenSocket->listen()) {
            std::println("PollServer: 监听套接字创建成功");
        }
        for (auto &[fd, events, revents]: _fdEvents) {
            fd = -1;
            events = revents = 0;
        }
        // 把监听套接字添加到辅助数组
        _fdEvents[0].fd = _listenSocket->getSockfd();
        _fdEvents[0].events = POLLIN;
        _fdEvents[0].revents = 0;
    }

    void dispatcher() {
        while (true) {
            constexpr int timeout = 2000;
            // 监听所关注的文件描述符是否“就绪”，即是否有数据可读
            int n = ::poll(_fdEvents, _size, timeout);
            if (n < 0) {
                std::println("PollServer: poll失败");
                continue;
            }
            std::println("PollServer: 接收到新连接， 数量为 {}", n);
            for (auto &[fd, event, revent]: _fdEvents) {
                if (fd == -1) {
                    continue;
                }
                if (revent & POLLIN) {
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

    ~PollServer() = default;

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listenSocket;
    pollfd _fdEvents[_size];
};
