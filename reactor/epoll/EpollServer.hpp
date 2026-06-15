#pragma once

#include <sys/epoll.h>

#include <print>

#include "TcpSockte.hpp"
#include "Log.hpp"


class EpollServer {
private:
    static constexpr int num = 1024;
    static constexpr int timeout = 2000;

    // 连接管理器
    void accepter(int fd) const {
        InetAddr clientAddr;
        const int linkFd = TcpSocket::accept(_listenSocket->getSockfd(), clientAddr);
        if (linkFd < 0) {
            LOG_ERROR() << "accept 失败: " << strerror(errno);
            return;
        }

        epoll_event event{};
        event.data.fd = linkFd;
        event.events = EPOLLIN;

        if (::epoll_ctl(_epFd, EPOLL_CTL_ADD, linkFd, &event) < 0) {
            LOG_ERROR() << std::format("epoll_ctl ADD 失败, fd: {}, error: {}", linkFd, strerror(errno));
            ::close(linkFd);
            return;
        }

        LOG_INFO() << std::format("获取一个新连接，fd: {}, 当前epoll监控中", linkFd);
    }


    void ioHandler(int fd) const {
        LOG_DEBUG() << std::format("处理客户端数据，文件描述符为 {}", fd);

        char buffer[1024];
        if (const ssize_t n = recv(fd, buffer, sizeof buffer - 1, 0); n == 0) {
            LOG_INFO() << std::format("连接断开，文件描述符为 {}", fd);

            if (::epoll_ctl(_epFd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
                LOG_WARN() << std::format("epoll_ctl DEL 失败, fd: {}, error: {}", fd, strerror(errno));
            } else {
                LOG_DEBUG() << std::format("已从epoll移除fd: {}", fd);
            }

            ::close(fd);
        } else if (n < 0) {
            LOG_ERROR() << std::format("读取数据失败，文件描述符为 {}, error: {}", fd, strerror(errno));

            if (::epoll_ctl(_epFd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
                LOG_WARN() << std::format("epoll_ctl DEL 失败, fd: {}, error: {}", fd, strerror(errno));
            }
            ::close(fd);
        } else {
            buffer[n] = '\0';
            LOG_INFO() << std::format("收到数据 [fd: {}, 长度: {}]: {}", fd, n, buffer);
        }
    }

    void eventsHandler(const epoll_event *revs, const int readyNum) const {
        LOG_DEBUG() << std::format("检测到 {} 个就绪事件", readyNum);

        for (int i = 0; i < readyNum; ++i) {
            if (const uint32_t events = revs[i].events; events & EPOLLIN) {
                if (const int fd = revs[i].data.fd;
                    fd == _listenSocket->getSockfd()) {
                    LOG_DEBUG() << std::format("监听套接字就绪，准备accept");
                    accepter(fd);
                    } else {
                        LOG_DEBUG() << std::format("客户端套接字就绪，fd: {}", fd);
                        ioHandler(fd);
                    }
            } else if (events & EPOLLERR) {
                LOG_ERROR() << std::format("epoll错误事件，fd: {}", revs[i].data.fd);
            } else if (events & EPOLLHUP) {
                LOG_WARN() << std::format("连接挂起，fd: {}", revs[i].data.fd);
            }
        }
    }

public:
    explicit EpollServer(const uint16_t port)
          : _port(port), _listenSocket(std::make_unique<TcpSocket>(port)), _epFd(::epoll_create1(EPOLL_CLOEXEC)) {
        if (_epFd < 0) {
            LOG_FATAL() << std::format("epoll_create1 失败: {}", strerror(errno));
            exit(1);
        }
        LOG_INFO() << std::format("EpollServer: epoll实例创建成功, epfd: {}", _epFd);

        _listenSocket->bind();
        if (!_listenSocket->listen()) {
            LOG_FATAL() << "监听套接字创建失败";
            exit(1);
        }
        LOG_INFO() << std::format("PollServer: 监听套接字创建成功, 端口: {}", port);

        epoll_event event{};
        event.data.fd = _listenSocket->getSockfd();
        event.events = EPOLLIN;

        if (epoll_ctl(_epFd, EPOLL_CTL_ADD, _listenSocket->getSockfd(), &event) < 0) {
            LOG_FATAL() << std::format("epoll_ctl ADD 监听套接字失败: {}", strerror(errno));
            exit(1);
        }
        LOG_INFO() << std::format("已将监听套接字添加到epoll, fd: {}", _listenSocket->getSockfd());
    }

    void start() const {
        LOG_INFO() << "EpollServer 开始运行...";

        epoll_event revs[num];

        while (true) {
            if (const int n = epoll_wait(_epFd, revs, num, timeout); n > 0) {
                LOG_DEBUG() << std::format("epoll_wait返回，就绪事件数: {}", n);
                eventsHandler(revs, n);
            } else if (n < 0) {
                LOG_ERROR() << std::format("epoll_wait失败: {}", strerror(errno));
            } else {}
        }
    }

    ~EpollServer() = default;

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listenSocket;
    int _epFd;
};
