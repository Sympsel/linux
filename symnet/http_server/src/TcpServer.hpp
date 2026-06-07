#pragma once

#include <cstring>
#include <csignal>
#include <memory>

#include "../../utils/module/SymNet.h"

template<class TaskType>
class TcpServer {
private:
    void HandleIO(const std::shared_ptr<ITcpSocket> &sockfd, const InetAddr &client_addr) {
        std::string inbuffer;
        if (const ssize_t n = sockfd->Recv(inbuffer); n < 0) {
            LOG_ERROR() << "recv error: " << strerror(errno);
        } else if (n == 0) {
            LOG_INFO() << "client closed the connection";
        } else {
            const std::string result = _task(inbuffer);
            // 将处理过的数据发回客户端
            if (const bool ret_status = sockfd->Send(result); ret_status == false) {
                LOG_ERROR() << "send error: " << strerror(errno);
            } else {
                LOG_INFO() << "send " << result.size() << " bytes to "
                        << client_addr.GetIp() << ":" << client_addr.GetPort();
            }
        }
    }

public:
    explicit TcpServer(const in_port_t port)
        : _port(port),
          _listen_sockfd(std::make_unique<TcpSocket>()),
          _task(nullptr) {
        _listen_sockfd->BuildServerSocketMethod(_port);
    }

    void Run(const TaskType &task) {
        // 需要运行的服务,例如HttpServer
        _task = task;
        while (true) {
            signal(SIGCHLD, SIG_IGN);

            InetAddr client_addr;
            auto sockfd = _listen_sockfd->Acceptor(client_addr);
            if (sockfd == nullptr) {
                LOG_ERROR() << "accept failed, continuing...";
                continue;
            }

            LOG_DEBUG() << "get a new link: [" << client_addr.GetIp() << ":" << client_addr.
                    GetPort() << ", " << "conn_sockfd: " << sockfd->GetSockfd() << "]";

            const pid_t pid = fork();
            if (pid < 0) {
                LOG_FATAL() << "fork error: " << strerror(errno);
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                _listen_sockfd->Close();
                HandleIO(sockfd, client_addr);
                sockfd->Close();
                exit(EXIT_SUCCESS);
            }
            sockfd->Close();
        }
    }

private:
    int _port;
    std::unique_ptr<ITcpSocket> _listen_sockfd;

    TaskType _task;
};
