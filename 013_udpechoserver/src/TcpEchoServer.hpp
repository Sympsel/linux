#pragma once

#include <netinet/in.h>

#include <csignal>
#include <functional>

#include "ThreadPool.hpp"
#include "Log.hpp"
#include "TcpSocket.hpp"

static in_port_t default_port = 8080;
static int default_x = 32;

using task_t = std::function<void()>;

class TcpEchoServer {
private:
    void Server(const TcpSocket &client_socket) const {
        char name[128];
        pthread_getname_np(pthread_self(), name, sizeof name);


        while (true) {
            char inbuffer[1024];
            ssize_t n = read(client_socket.GetSockfd(), inbuffer, sizeof inbuffer);
            if (n > 0) {
                inbuffer[n] = '\0';
                LOG_DEBUG() << "recv: " << inbuffer;
            } else if (n == 0) {
                LOG_DEBUG() << "client closed, [fd: " << client_socket.GetSockfd() << "]";
                break;
            } else {
                LOG_ERROR() << "read error";
                break;
            }

            // handle data
            std::string echo_str = "echo: ";
            echo_str.append(inbuffer);

            n = write(client_socket.GetSockfd(), echo_str.c_str(), echo_str.size());
            if (n < 0) {
                LOG_ERROR() << "write error";
            }
        }
    }

public:
    explicit TcpEchoServer(
        const in_port_t port = default_port
    )
        : _listen_sockfd(-1), _port(port) {
    }

    void Init() {
        // create socket
        TcpSocket listen_socket(_port);
        // bind
        listen_socket.Bind();
        // listen
        TcpSocket::Listen(listen_socket.GetSockfd(), default_x);

        // accept
    }

    void Start() const {
        // ignore child
        while (true) {
            TcpSocket client_socket{};
            int conn_sockfd = client_socket.Accept();
            if (conn_sockfd < -1) continue;
            LOG_INFO() << "get a new link: [" << client_socket.GetAddr().GetIp() << ":" << client_socket.GetAddr().
                    GetPort() << ", " << "conn_sockfd: " << conn_sockfd << "]";

            ThreadPool<task_t>::GetInstance().Enqueue([client_socket, this] {
                Server(client_socket);
            });
        }
    }

    ~TcpEchoServer() {
        close(_listen_sockfd);
    }

private:
    int _listen_sockfd;
    in_port_t _port;
};
