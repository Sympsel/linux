#pragma once

#include "SocketUtils.hpp"
#include "TcpSocket.hpp"

/**
 * @brief tcp server socket
 */
class TcpSSocket {
public:
    /**
     * @param port port
     * @param ip default: 0.0.0.0 to bind any client ip
     */
    explicit TcpSSocket(const in_port_t &port, const std::string &ip = "0.0.0.0") : _addr_helper{port, ip} {
    }

    explicit TcpSSocket(const sockaddr_in &addr) {
        _addr_helper.SetAddr(addr);
    }

    explicit TcpSSocket(const InetAddr &inet_addr) : _addr_helper(inet_addr) {
    }


    void Bind() const {
        if (const int ret = bind(_socket.GetSockfd(), reinterpret_cast<const sockaddr *>(&_addr_helper.GetAddr()),
                                 _addr_helper.GetAddrLen()); ret < 0) {
            LOG_FATAL() << "bind error";
            exit(1);
        }
        LOG_INFO() << "bind success. [fd: " << _socket.GetSockfd() << "]";
    }

    /**
     * @warning this will overwrite _addr_helper with client address.
     *          if you need to preserve local address, use static Accept(sockfd, client_addr) instead.
     */
    int Accept(InetAddr& client_addr) const {
        return TcpSocket::Accept(_socket.GetSockfd(), client_addr);
    }

    [[nodiscard]] bool Listen(const int backlog) const {
        return TcpSocket::Listen(_socket.GetSockfd(), backlog);
    }

    [[nodiscard]] const InetAddr &GetAddr() const {
        return _addr_helper;
    }

    [[nodiscard]] const int &GetSockfd() const {
        return _socket.GetSockfd();
    }

    ~TcpSSocket() = default;

private:
    TcpSocket _socket;
    InetAddr _addr_helper;
};
