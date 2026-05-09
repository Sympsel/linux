#pragma once

#include <netinet/in.h>

#include "../module/SymINet.h"
#include "../module/SymLog.h"

static constexpr in_port_t default_port = 8080;
static constexpr int default_backlog = 32;
static constexpr int default_thread_name_length = 128;
static constexpr int default_buffer_length = 1024;

template<class TaskType>
class TcpServer {
private:
    void HandleClient(const int client_sockfd, const InetAddr &client_addr) const {
        while (true) {
            std::string command = SocketUtils::Read(client_sockfd);
            if (command.empty()) {
                LOG_INFO() << "client closed, [fd: " << client_sockfd << ", addr: "
                        << client_addr.GetIp() << ":" << client_addr.GetPort() << "]";
                break;
            }
            LOG_DEBUG() << "recv from [" << client_addr.GetIp() << ":" << client_addr.GetPort() << "]: " << command;

            std::string result;
            if (_task == nullptr) {
                LOG_ERROR() << "task is not set";
                result = "server error, it's not your fault";
            } else {
                result = _task(command);
            }

            if (const ssize_t n = SocketUtils::Write(client_sockfd, result); n < 0) {
                LOG_ERROR() << "write error to fd " << client_sockfd << ": " << strerror(errno);
                break;
            }
        }
        close(client_sockfd);
        LOG_INFO() << "connection closed, fd: " << client_sockfd;
    }

public:
    explicit TcpServer(const in_port_t port = default_port)
        : _listen_socket(port), _port(port), _task(nullptr) {
    }

    void Init(const TaskType& task) {
        _listen_socket.Bind();
        if (!_listen_socket.Listen(default_backlog)) {
            LOG_FATAL() << "failed to listen on port " << _port;
            exit(EXIT_FAILURE);
        }
        _task = task;
        LOG_INFO() << "server initialized on port" << _port;
    }

    void Start() const {
        LOG_INFO() << "server starting, waiting for connection";
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            InetAddr client_addr;
            const int conn_sockfd = _listen_socket.Accept(client_addr);
            if (conn_sockfd < 0) {
                LOG_ERROR() << "accept failed, continuing...";
                continue;
            }

            LOG_INFO() << "get a new link: [" << client_addr.GetIp() << ":" << client_addr.
                    GetPort() << ", " << "conn_sockfd: " << conn_sockfd << "]";

            if (const int pid = fork(); pid < 0) {
                LOG_ERROR() << "fork error: " << strerror(errno);
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                if (const int grandchild_pid = fork(); grandchild_pid > 0) {
                    close(conn_sockfd);
                    exit(EXIT_SUCCESS);
                } else if (grandchild_pid == 0) {
                    HandleClient(conn_sockfd, client_addr);
                    exit(EXIT_SUCCESS);
                } else {
                    LOG_ERROR() << "fork error: " << strerror(errno);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    ~TcpServer() = default;

private:
    TcpSSocket _listen_socket;
    in_port_t _port;
    TaskType _task;
};
