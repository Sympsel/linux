#pragma once

#include <functional>
#include <memory>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "TcpConnection.hpp"

class TcpServer {
    using NewConnectionCallback = std::function<void(std::shared_ptr<TcpConnection>)>;

public:
    TcpServer(const int port) : _port(port), _listen_fd(-1), _epoll_fd(-1) {
    }

    bool start() {
        // 创建监听套接字
        _listen_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (_listen_fd < 0) {
            LOG_FATAL() << "Failed to create listen socket";
            return false;
        }

        // 设置地址重用
        constexpr int opt = 1;
        ::setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);

        // 绑定地址
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(_port);

        if (::bind(_listen_fd, reinterpret_cast<const sockaddr *>(&addr), sizeof addr) < 0) {
            LOG_FATAL() << "Failed to bind address";
            return false;
        }

        // 监听
        if (::listen(_listen_fd, 128) < 0) {
            LOG_ERROR() << "Failed to listen";
            return false;
        }

        // 创建epoll
        _epoll_fd = ::epoll_create1(0);
        if (_epoll_fd < 0) {
            LOG_ERROR() << "Failed to create epoll";
            return false;
        }

        // 添加监听套接字到epoll
        epoll_event event{};
        event.events = EPOLLIN; // 监听可读事件
        event.data.fd = _listen_fd;
        ::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _listen_fd, &event);

        return true;
    }

    void run() {
        struct epoll_event events[1024];
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            // 等待事件
            const int nfds = ::epoll_wait(_epoll_fd, events, 1024, -1);

            for (int i{}; i < nfds; ++i) {
                if (events[i].data.fd == _listen_fd) {
                    // 接收新连接
                    sockaddr_in client_addr{};
                    socklen_t client_len = sizeof client_addr;
                    int client_fd = ::accept4(_listen_fd,
                                              reinterpret_cast<struct sockaddr *>(&client_addr),
                                              &client_len, SOCK_NONBLOCK);
                    if (client_fd < 0) {
                        LOG_WARN() << "Failed to accept new connection";
                        continue;
                    }
                    // 添加到epoll
                    const auto conn = std::make_shared<TcpConnection>(client_fd);
                    _connections[client_fd] = conn;

                    // 将新连接加入epoll
                    epoll_event event{};
                    event.events = EPOLLIN | EPOLLET; // 边缘触发模式
                    event.data.fd = client_fd;
                    ::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

                    if (_new_connection_cb) {
                        _new_connection_cb(conn);
                    } else {
                        LOG_WARN() << "未注册创建新连接回调";
                    }
                } else {
                    // 处理已有连接可读事件
                    int fd = events[i].data.fd;
                    if (auto it = _connections.find(fd); it != _connections.end()) {
                        it->second->handleRead();
                    } else {
                        LOG_WARN() << "未找到对应的连接";
                    }
                }
            }
        }
    }

    void setNewConnectionCallback(NewConnectionCallback cb) {
        _new_connection_cb = std::move(cb);
    }

private:
    int _port;
    int _listen_fd;
    int _epoll_fd;
    std::unordered_map<int, std::shared_ptr<TcpConnection> > _connections;
    // 新连接回调
    NewConnectionCallback _new_connection_cb;
};
