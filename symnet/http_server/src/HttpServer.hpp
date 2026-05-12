#pragma once

#include <iostream>
#include <netinet/in.h>
#include "TcpServer.hpp"
#include "HttpProtocol.hpp"
#include "HttpSerializer.hpp"

template <class TaskType>
class HttpServer {
public:
    explicit HttpServer(in_port_t port) :
    _port(port),
    _tcp_server(std::make_unique<TcpServer<TaskType>>(port)){

    }

    std::string HandlerHttpRequire(const std::string& stream_str) {
        LOG_DEBUG() << "handle http request: " << stream_str;
        return stream_str;
    }

    void Run() {
        _tcp_server->Run();
    }

    ~HttpServer() = default;
private:
    in_port_t _port;
    TaskType _task;
    // todo replace
    std::unique_ptr<TcpServer<TaskType>> _tcp_server;
    std::unique_ptr<HttpProtocol<TaskType, HttpSerializer>> _http_server;
};
