#include "Listener.h"

#include "TcpConnection.h"
#include "Reactor.hpp"

void Listener::accept() const {
    while (true) {
        InetAddr clientAddr;
        const int clientSockFd = TcpSocket::accept(_listenSocket->getSockfd(), clientAddr);
        if (clientSockFd < 0) {
            // 没有新连接
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            // 被信号打断
            if (errno == EINTR) {
                continue;
            }
            LOG_ERROR() << std::format("accept 出错，errno={}", errno);
            return;
        }
        LOG_DEBUG() << std::format("获得一个新连接, 客户端套接字：{}", clientSockFd);
        // 使用工厂函数创建连接对象
        const std::shared_ptr<Connection> clientConnect = _connectionFactory(clientSockFd);
        clientConnect->setInetAddr(clientAddr);
        setNonBlock(clientSockFd);
        clientConnect->setReadWriteEventItem(true, false);
        if (_reactor) {
            _reactor->addConnection(clientConnect);
        }
    }
}
