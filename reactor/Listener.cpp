#include "IOHandler.h"
#include "Reactor.hpp"

void Listener::receiver() {
    while (true) {
        InetAddr clientAddr;
        int clientSockFd = TcpSocket::accept(_listenSocket->getSockfd(), clientAddr);
        if (clientSockFd < 0) {
            // 没有新连接
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            // 被信号打断
            if (errno == EINTR) {
                LOG_INFO() << "被信号中断";
                continue;
            }
            LOG_ERROR() << std::format("accept 返回错误，errno={}", errno);
            expecter();
        } else {
            LOG_DEBUG() << std::format("PollServer: 获得一个新连接, 客户端套接字：{}", clientSockFd);
            const std::shared_ptr<Connection> clientConnect = std::make_shared<IOHandler>(clientSockFd);
            clientConnect->setInetAddr(clientAddr);
            setNonBlock(clientSockFd);
            clientConnect->setEventItem(IN | ET);
            if (_reactor) {
                _reactor->addConnection(clientConnect);
            }
        }
    }
}
