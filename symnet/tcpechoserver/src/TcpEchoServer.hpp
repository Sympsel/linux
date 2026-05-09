#pragma once

#include <functional>

#include "../../utils/module/SymINet.h"
#include "../../utils/module/SymLog.h"
#include "../../utils/module/SymThread.h"

static constexpr in_port_t default_port = 8080;
static constexpr int default_backlog = 32;
static constexpr int default_thread_name_length = 128;
static constexpr int default_buffer_length = 1024;

using task_t = std::function<void()>;

class TcpEchoServer {
private:
    static void HandleClient(const int client_sockfd, const InetAddr &client_addr) {
        char thread_name[default_thread_name_length];
        pthread_getname_np(pthread_self(), thread_name, sizeof thread_name);

        LOG_INFO() << "Thread [" << thread_name << "] handling client "
                << client_addr.GetIp() << ":" << client_addr.GetPort();


        while (true) {
            char inbuffer[default_buffer_length];

            if (const ssize_t n = read(client_sockfd, inbuffer, sizeof inbuffer - 1); n > 0) {
                inbuffer[n] = '\0';
                LOG_DEBUG() << "recv from [" << client_addr.GetIp() << ":"
                        << client_addr.GetPort() << "]: " << inbuffer;
                std::string echo_str = "echo: ";
                echo_str.append(inbuffer);

                if (const ssize_t m = write(client_sockfd, echo_str.c_str(), echo_str.size()); m < 0) {
                    LOG_ERROR() << "write error to fd " << client_sockfd << ": " << strerror(errno);
                    break;
                }
            } else if (n == 0) {
                LOG_INFO() << "client closed, [fd: " << client_sockfd
                        << ", addr: " << client_addr.GetIp() << ":" << client_addr.GetPort() << "]";
                break;
            } else {
                LOG_ERROR() << "read error from fd " << client_sockfd << ": " << strerror(errno);
                break;
            }
        }
        close(client_sockfd);
        LOG_INFO() << "connection closed, fd: " << client_sockfd;
    }

public:
    explicit TcpEchoServer(const in_port_t port = default_port)
        : _listen_socket(port), _port(port) {
    }

    void Init() const {
        _listen_socket.Bind();
        if (!_listen_socket.Listen(default_backlog)) {
            LOG_FATAL() << "failed to listen on port " << _port;
            exit(EXIT_FAILURE);
        }

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

            ThreadPool<task_t>::GetInstance().Enqueue([conn_sockfd, client_addr] {
                HandleClient(conn_sockfd, client_addr);
            });
        }
    }

    ~TcpEchoServer() = default;

private:
    TcpSSocket _listen_socket;
    in_port_t _port;
};
