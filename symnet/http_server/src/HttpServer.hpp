#pragma once

#include <netinet/in.h>
#include "TcpServer.hpp"
#include "HttpProtocol.hpp"

using http_business_task_t = std::function<HttpResponse(const HttpRequest &)>;

class HttpServer {
public:
    using http_task_t = std::function<std::string(const std::string&)>;

    explicit HttpServer(in_port_t port) :
    _port(port),
    _tcp_server(std::make_unique<TcpServer<http_task_t>>(port)){
    }

    // 传递业务逻辑
    void Run(const http_business_task_t& task) {
        _task = task;
        _http_protocol = std::make_unique<HttpProtocol>(task);
        _tcp_server->Run(_http_protocol->GetHandler());
    }

    ~HttpServer() = default;
private:
    in_port_t _port;
    TaskType _task;
    std::unique_ptr<TcpServer<http_task_t>> _tcp_server;
    std::unique_ptr<HttpProtocol> _http_protocol;
};
