#include "../TcpConnection.h"
#include "../core/Reactor.hpp"
#include "TcpSocket.hpp"

void TcpConnection::receiver() {
    while (true) {
        const ssize_t n = TcpSocket::receive(_sockFd, _inBuffer);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            LOG_WARN() << std::format("客户端接收出错，地址为{}:{}"
                                      , _clientAddr.getIp(), _clientAddr.getPort());
            handleClose();
            return;
        }
        if (n == 0) {
            LOG_INFO() << std::format("客户端退出，地址为{}:{}"
                                      , _clientAddr.getIp(), _clientAddr.getPort());
            handleClose();
            return;
        }
    }

    // 如果有自定义消息处理器，调用它
    if (!_inBuffer.empty()) {
        if (_messageHandler) {
            _messageHandler(_inBuffer, _outBuffer);
        } else {
            // 默认行为：回显
            _outBuffer.append(_inBuffer.peek(), _inBuffer.getReadableSize());
            _inBuffer.retrieveAll();
        }

        // 触发发送：设置 OUT 事件
        if (_reactor && !_outBuffer.empty()) {
            _reactor->setReadWriteEventItem(_sockFd, true, true);
        }
    }
}

void TcpConnection::sender() {
    if (_reactor == nullptr) {
        LOG_ERROR() << "没有设置反应堆";
    }
    if (_outBuffer.empty()) {
        // 没有数据要发送，移除 OUT 事件
        _reactor->setReadWriteEventItem(_sockFd, true, false);
        return;
    }

    while (!_outBuffer.empty()) {
        if (ssize_t n = TcpSocket::sendTo(_sockFd, _outBuffer);
            n > 0) {
            _outBuffer.retrieve(n);
            LOG_DEBUG() << std::format("发送数据，长度: {}", n);
        } else if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 发送缓冲区满了，等待下次 OUT 事件
                break;
            }
            if (errno == EINTR) {
                // 被信号中断，重试
                continue;
            }
            LOG_ERROR() << std::format("发送数据失败，地址为{}:{}，错误信息：{}",
                                       _clientAddr.getIp(), _clientAddr.getPort(), strerror(errno)
            );
            handleClose();
            return;
        }
    }

    if (_outBuffer.empty()) {
        _reactor->setReadWriteEventItem(_sockFd, true, false);
    }
}

void TcpConnection::expecter() {
    LOG_INFO() << std::format("清理连接，地址为{}:{}", _clientAddr.getIp(), _clientAddr.getPort());
    handleClose();
}

void TcpConnection::handleClose() {
    if (_reactor) {
        _reactor->removeConnection(std::shared_ptr<TcpConnection>(this));
    }
}
