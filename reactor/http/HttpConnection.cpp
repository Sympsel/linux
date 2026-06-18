#include "HttpConnection.h"

#include "Reactor.hpp"
#include "TcpSocket.hpp"

void HttpConnection::receiver() {
    while (true) {
        const auto n = TcpSocket::receive(_sockFd, _inBuffer);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 数据读完
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            LOG_ERROR() << std::format("接收报文出错, errno={}", errno);
            handleClose();
            return;
        }
        if (n == 0) {
            LOG_INFO() << std::format("客户端 {} 关闭连接", _sockFd);
            handleClose();
            return;
        }

        if (!_inBuffer.empty()) {
            if (parseRequest()) {
                handleRequest();
                sendResponse();
            }
            _inBuffer.retrieveAll();
        }
    }
}

void HttpConnection::sender() {
    while (!_outBuffer.empty()) {
        const ssize_t n = TcpSocket::sendTo(_sockFd, _outBuffer);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            LOG_ERROR() << std::format("报文发送出错, errno={}", errno);
            handleClose();
            return;
        }
        _outBuffer.retrieve(n);
    }

    if (_outBuffer.empty() && _reactor) {
        _reactor->setReadWriteEventItem(_sockFd, true, false);
    }
}

void HttpConnection::expecter() {
    handleClose();
}

bool HttpConnection::parseRequest() {
    if (const auto result = _httpRequest->parse(_inBuffer);
        result.has_value()) {
        LOG_DEBUG() << std::format("解析 HTTP 请求成功: {} {}",
                                   _httpRequest->methodToString(),
                                   _httpRequest->getArg("path"));
        return true;
    }
    LOG_WARN() << "HTTP 请求解析失败，缓冲区大小: " << _inBuffer.getReadableSize();
    return false;
}

void HttpConnection::handleRequest() const {
    if (_requestHandler) {
        _requestHandler(*_httpRequest, *_httpResponse);
    } else {
        _httpResponse->_msg.statusCode = 200;
        _httpResponse->_msg.statusText = "OK";
        _httpResponse->_msg.headers["Content-Type"] = "text/html";
        _httpResponse->_msg.body = "<h1>服务端出错，这不是你的原因</h1>";
    }
}

void HttpConnection::sendResponse() {
    const std::string response = _httpResponse->serialize();
    _outBuffer.append(response);

    if (_reactor) {
        _reactor->setReadWriteEventItem(_sockFd, true, true);
    }
}

void HttpConnection::handleClose() {
    if (_reactor) {
        _reactor->removeConnection(std::shared_ptr<HttpConnection>(this));
    }
}
