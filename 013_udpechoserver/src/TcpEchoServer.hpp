#pragma once

#include <netinet/in.h>
#include "Log.hpp"
#include "TcpSocket.hpp"

static in_port_t default_port = 8080;
static int default_x = 32;

class TcpEchoServer {
private:
    // void Server(int sockfd, TcpSocket) {
        // todo
    // }
public:
    explicit TcpEchoServer(const in_port_t port = default_port) : _listen_sockfd(-1), _port(port) {
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
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            TcpSocket client{};
            int conn_sockfd = client.Accept();
            if (conn_sockfd < 0) continue;
            LOG_INFO() << "get a new link: [" << client.GetAddr().GetIp() << ":" << client.GetAddr().GetPort() << ", "
                    << "conn_sockfd: " << conn_sockfd << "]";
        }
    }

    ~TcpEchoServer() {
        close(_listen_sockfd);
    }

private:
    int _listen_sockfd;
    in_port_t _port;
};
