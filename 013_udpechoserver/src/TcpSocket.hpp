#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "InetAddr.hpp"
#include "Log.hpp"

class TcpSocket {
public:
    /**
     * @warning you should initialize it before use
     */
    TcpSocket() = default;

    /**
     * @param port port
     * @param ip default: 0.0.0.0 to bind any client ip
     */
    explicit TcpSocket(const in_port_t& port, const std::string &ip = "0.0.0.0") : _addr_helper{port, ip} {
    }

    explicit TcpSocket(const sockaddr_in& addr) {
        _addr_helper.SetAddr(addr);
    }

    explicit TcpSocket(const InetAddr &inet_addr) : _addr_helper(inet_addr) {
    }

    static int GetSockfd() {
        const int sockfd = socket(AF_INET, SOCK_STREAM, 0); // IPPROTO_TCP
        if (sockfd < 0) {
            LOG_FATAL() << "socket error";
            exit(2);
        }
        LOG_INFO() << "socket success. [fd: " << sockfd << "]";
        return sockfd;
    }

    void Bind(const int sockfd) {
        sockaddr_in temp{_addr_helper.GetAddr()};
        if (const int ret = bind(sockfd, reinterpret_cast<sockaddr*>(&temp), sizeof(temp)); ret < 0) {
            LOG_FATAL() << "bind error";
            exit(1);
        }
        LOG_INFO() << "bind success. [fd: " << sockfd << "]";
    }

    static void Listen(const int sockfd, const int backlog = 32) {
        if (const int ret = listen(sockfd, backlog); ret < 0) {
            LOG_FATAL() << "listen error";
            exit(1);
        }
    }

    static int Accept(const int sockfd, InetAddr& client_addr) {
        sockaddr_in temp{};
        socklen_t addrlen = sizeof(temp);

        const int conn_sockfd = accept(sockfd, reinterpret_cast<sockaddr*>(&temp), &addrlen);
        if (conn_sockfd < 0) {
            LOG_WARN() << "accept error";
            return -1;
        }
        client_addr.SetAddr(temp);
        LOG_INFO() << "accept success. [fd: " << conn_sockfd << "]";
        return conn_sockfd;
    }

    [[nodiscard]] InetAddr GetAddr() const {
        return _addr_helper;
    }

    ~TcpSocket() = default;
private:
    InetAddr _addr_helper;
};
