#pragma once

#include <netinet/in.h>
#include "TcpServer.hpp"
#include "HttpProtocol.hpp"

template <class TaskType>
class HttpServer {
public:
    using http_task_t = std::function<std::string(const std::string&)>;

    explicit HttpServer(in_port_t port) :
    _port(port),
    _tcp_server(std::make_unique<TcpServer<http_task_t>>(port)){
    }

    void Run(const TaskType& task) {
        _task = task;
        _http_protocol = std::make_unique<HttpProtocol<TaskType>>(task);
        _tcp_server->Run(_http_protocol->GetHandler());
    }

    ~HttpServer() = default;
private:
    in_port_t _port;
    TaskType _task;
    std::unique_ptr<TcpServer<http_task_t>> _tcp_server;
    std::unique_ptr<HttpProtocol<TaskType>> _http_protocol;
};
