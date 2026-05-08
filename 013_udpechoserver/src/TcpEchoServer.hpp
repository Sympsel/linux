#pragma once

#include <netinet/in.h>
#include "Log.hpp"
#include "TcpSocket.hpp"

#include "csignal"

static in_port_t default_port = 8080;
static int default_x = 32;

class TcpEchoServer {
private:
    void Server(const TcpSocket &client_socket) const {
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
        signal(SIGCHLD, SIG_IGN);
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            TcpSocket client_socket{};
            int conn_sockfd = client_socket.Accept();
            if (conn_sockfd < -1) continue;
            LOG_INFO() << "get a new link: [" << client_socket.GetAddr().GetIp() << ":" << client_socket.GetAddr().
                    GetPort() << ", " << "conn_sockfd: " << conn_sockfd << "]";
            if (const pid_t pid = fork(); pid < 0) {
                LOG_FATAL() << "fork error";
                exit(-1);
            } else if (pid == 0) {
                Server(client_socket);
                exit(0);
            } else {
            }
        }
    }

    ~TcpEchoServer() {
        close(_listen_sockfd);
    }

private:
    int _listen_sockfd;
    in_port_t _port;
};
