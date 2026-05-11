#pragma once

#include <cstring>
#include <csignal>
#include <memory>

#include "../../utils/module/SymNet.h"

template<class TaskType>
class TcpServer {
private:
    void HandleIO(const std::shared_ptr<ITcpSocket>& sockfd, const InetAddr& client_addr) {
        std::string inbuffer;
        while (true) {
            if (const ssize_t n = sockfd->Recv(inbuffer); n < 0) {
                LOG_WARN() << "recv error: " << strerror(errno);
                break;
            } else if (n == 0) {
                LOG_INFO() << "client closed, [fd: " << sockfd->GetSockfd() << ", addr: "
                        << client_addr.GetIp() << ":" << client_addr.GetPort() << "]";
                break;
            }

            // handle request
            std::string out_buffer;
            if (_task == nullptr) {
                LOG_WARN() << "task is not set";
                out_buffer = "server error, it's not your fault";
            } else {
                out_buffer = _task(inbuffer);
            }

            sockfd->Send(out_buffer);
        }
    }
public:
    explicit TcpServer(const in_port_t port)
        : _port(port),
          _listen_sockfd(std::make_unique<TcpSocket>()),
          _running(false),
          _task(nullptr) {
        _listen_sockfd->BuildServerSocketMethod(_port);
    }

    void Init(const TaskType& task) {
        _task = task;
    }

    void Run() {
        _running = true;
        // ReSharper disable once CppDFAConstantConditions
        while (_running) {
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
    bool _running;

    TaskType _task;
};
