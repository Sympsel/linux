#pragma once
#include <functional>
#include <memory>

#include "HttpData.hpp"
#include "../tcp/TcpServer.hpp"
#include "HttpParser.hpp"

class HttpServer {
private:
    using HttpCallback = std::function<void(const HttpRequest &, HttpResponse &)>;

public:
    HttpServer(const int port)
        : _port(port),
          _tcp_server(std::make_unique<TcpServer>(port)) {
        _tcp_server->setNewConnectionCallback(
            [this](const std::shared_ptr<TcpConnection> &conn) {
                onNewConnection(conn);
            }
        );
    }

    void start() const {
        if (_tcp_server->start()) {
            LOG_INFO() << "HttpServer started on port " << _port;
            _tcp_server->run();
        } else {
            LOG_ERROR() << "Failed to start HttpServer";
        }
    }

    // 应用层注册业务处理回调
    void setHttpCallback(const HttpCallback& cb) {
        _http_cb = cb;
    }

private:
    void onNewConnection(const std::shared_ptr<TcpConnection> &conn) const {
        // 为每个连接设置消息处理回调
        conn->setMessageCallback(
            [this](const std::shared_ptr<TcpConnection> &connection, Buffer &buffer) {
                onMessage(connection, buffer);
            }
        );

        conn->setCloseCallback(
            [this](const std::shared_ptr<TcpConnection> &connection) {
                onClose(connection);
            }
        );
    }

    void onMessage(const std::shared_ptr<TcpConnection> &connection, Buffer &buffer) const {
        HttpRequest request;

        // 反序列化: 从buffer中反序列化出HttpRequest对象
        if (HttpParser::parse(buffer, request)) {
            HttpResponse response;

            // 调用应用层业务
            if (_http_cb) {
                _http_cb(request, response);
            } else {
                LOG_WARN() << "No http callback";
            }

            // 序列化: 将HttpResponse对象序列化成字符串
            const std::string resp_str = response.serialize();

            // 发送
            connection->send(resp_str);
        } else {
            LOG_ERROR() << "HttpParser::parse failed";
        }
    }

    void onClose(const std::shared_ptr<TcpConnection> &connection) const {
        LOG_INFO() << "Connection closed: " << connection->getSockfd();
        // 用一下成员变量, 防止编译器报警
        (void) _port;
    }

    int _port;
    std::unique_ptr<TcpServer> _tcp_server;
    // 应用层业务处理回调
    HttpCallback _http_cb;
};
