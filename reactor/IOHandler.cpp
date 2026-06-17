#include "IOHandler.h"
#include "Reactor.hpp"

void IOHandler::receiver() {
    while (true) {
        const ssize_t n = TcpSocket::receive(_clientSockFd, _inBuffer);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            LOG_WARN() << std::format("客户端接收出错，地址为{}:{}"
                                      , _clientAddr.getIp(), _clientAddr.getPort());
            if (_reactor) {
                _reactor->removeConnection(shared_from_this());
            }
            expecter();
            break;
        }
        if (n == 0) {
            LOG_INFO() << std::format("客户端退出，地址为{}:{}"
                                      , _clientAddr.getIp(), _clientAddr.getPort());
            break;
        }
    }
    // 如果有完整报文，就从输入缓冲区切下来一个报文，如果没有就什么也不做
    if (!_inBuffer.empty()) {
        _outBuffer.append(handleRequest(_inBuffer));
        // 触发发送：设置 OUT 事件
        if (_reactor && !_outBuffer.empty()) {
            _reactor->updateReadWriteEventItem(_clientSockFd, true, true);
        }
    }
}

std::string IOHandler::handleRequest(Buffer &request) {
    size_t pos = 0;
    // while ((pos = request.find("\r\n", pos)) != std::string::npos) {
    //     request.replace(pos, 2, "\n");
    //     ++pos;
    // }
    // std::replace(request.begin(), request.end(), '\r', '\n');
    // todo 注册一个协议
    std::string response = request.peek();
    request.retrieveAll();
    LOG_DEBUG() << std::format("收到数据，长度: {}", response.size());
    return response;
}

void IOHandler::sender() {
    if (_reactor == nullptr) {
        LOG_ERROR() << "没有设置反应堆";
    }
    if (_outBuffer.empty()) {
        // 没有数据要发送，移除 OUT 事件
        _reactor->updateReadWriteEventItem(_clientSockFd, true, false);
    }
    while (!_outBuffer.empty()) {
        if (ssize_t n = TcpSocket::sendTo(_clientSockFd, _outBuffer);
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
            _reactor->removeConnection(shared_from_this());
            break;
        }
    }
    if (_outBuffer.empty()) {
        _reactor->updateReadWriteEventItem(_clientSockFd, true, false);
    }
}

void IOHandler::expecter() {
    LOG_INFO() << std::format("清理连接，地址为{}:{}", _clientAddr.getIp(), _clientAddr.getPort());
    if (_reactor) {
        _reactor->removeConnection(shared_from_this());
    } else {
        LOG_ERROR() << "没有设置反应堆";
    }
}
