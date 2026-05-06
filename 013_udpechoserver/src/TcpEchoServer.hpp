#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Log.hpp"
#include "TcpSocket.hpp"

static in_port_t default_port = 8080;
static int default_x = 32;

class TcpEchoServer {
public:
    TcpEchoServer(const in_port_t port = default_port) : _listen_sockfd(-1), _port(port) {
    }

    void Init() {
        // create socket
        _listen_sockfd = TcpSocket::GetSockfd();
        // bind
        TcpSocket local_addr(_port);
        local_addr.Bind(_listen_sockfd);
        // listen
        TcpSocket::Listen(_listen_sockfd, default_x);

        // accept
    }

    void Start() {
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            InetAddr temp;
            int conn_sockfd = TcpSocket::Accept(_listen_sockfd, temp);
            if (conn_sockfd < 0) continue;
            TcpSocket client{temp};
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
