#pragma once

#include "SocketUtils.hpp"
#include "TcpSocket.hpp"

/**
 * @brief TCP server socket wrapper class.
 *
 * Provides functionality for creating and managing TCP server sockets,
 * including binding, listening, and accepting client connections.
 */
class TcpSSocket {
public:
    /**
    * @brief Constructs a TCP server socket with specified port and optional IP.
    * @param port Port number to bind to
    * @param ip IP address to bind to (default: "0.0.0.0" for any interface)
    */
    explicit TcpSSocket(const in_port_t &port, const std::string &ip = Conf::network_default_bind_ip)
        : _addr_helper{port, ip} {
    }

    /**
     * @brief Constructs a TCP server socket from a sockaddr_in structure.
     * @param addr Address structure containing IP and port
     */
    explicit TcpSSocket(const sockaddr_in &addr) {
        _addr_helper.SetAddr(addr);
    }

    /**
     * @brief Constructs a TCP server socket from an InetAddr object.
     * @param inet_addr InetAddr object containing IP and port information
     */
    explicit TcpSSocket(const InetAddr &inet_addr) : _addr_helper(inet_addr) {
    }

    /**
     * @brief Binds the socket to the configured address.
     * @note Terminates the program if binding fails.
     */
    void Bind() const {
        if (const int ret = bind(_socket.GetSockfd(), reinterpret_cast<const sockaddr *>(&_addr_helper.GetAddr()),
                                 _addr_helper.GetAddrLen()); ret < 0) {
            LOG_FATAL() << "bind error";
            exit(1);
        }
        LOG_INFO() << "bind success. [fd: " << _socket.GetSockfd() << "]";
    }

    /**
     * @brief Accepts an incoming client connection.
     * @warning This method overwrites _addr_helper with the client address.
     *          To preserve the local address, use TcpSocket::Accept(sockfd, client_addr) instead.
     * @param client_addr Output parameter to store the connecting client's address
     * @return Connected socket file descriptor for communication with the client
     */
    int Accept(InetAddr &client_addr) const {
        return TcpSocket::Accept(_socket.GetSockfd(), client_addr);
    }

    /**
     * @brief Starts listening for incoming connections.
     * @param backlog Maximum number of pending connections in the queue
     * @return true if listening starts successfully, false otherwise
     */
    [[nodiscard]] bool Listen(const int backlog = Conf::network_backlog) const {
        return TcpSocket::Listen(_socket.GetSockfd(), backlog);
    }

    /**
     * @brief Gets the bound address information.
     * @return Reference to the InetAddr object containing address details
     */
    [[nodiscard]] const InetAddr &GetAddr() const {
        return _addr_helper;
    }

    /**
     * @brief Gets the socket file descriptor.
     * @return Reference to the socket file descriptor
     */
    [[nodiscard]] const int &GetSockfd() const {
        return _socket.GetSockfd();
    }

    ~TcpSSocket() = default;

private:
    TcpSocket _socket;
    InetAddr _addr_helper;
};
