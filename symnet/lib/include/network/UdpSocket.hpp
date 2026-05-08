#pragma once

#include "SocketUtils.hpp"
#include "../utils/InetAddr.hpp"
#include "../utils/Log.hpp"

#define CREATE_SOCKET (SocketUtils::CreateSocket(AF_INET, SOCK_DGRAM))

/**
 * @brief UDP socket wrapper class for datagram communication.
 *
 * Provides functionality for UDP socket creation, binding,
 * sending, and receiving datagrams with or without address information.
 */
class UdpSocket {
private:
    /**
     * @brief Validates that the socket file descriptor is valid.
     * @return true if socket is valid, false otherwise
     */
    bool CheckSockfd() const {
        if (_sockfd < 0) {
            LOG_ERROR() << "create UDP socket failed";
            return false;
        }
        return true;
    }

public:
    /**
     * @brief Constructs a basic UDP socket.
     */
    UdpSocket() : _sockfd(CREATE_SOCKET) {
        (void)CheckSockfd();
    }

    /**
     * @brief Constructs a UDP socket and binds it to a specific address (for UDP servers).
     * @param port Port number to bind to
     * @param ip IP address to bind to (default: "0.0.0.0" for any interface)
     */
    explicit UdpSocket(const in_port_t &port, std::string ip = "0.0.0.0") : _sockfd(CREATE_SOCKET) {
        _local_addr = InetAddr(port, std::move(ip));
        (void)CheckSockfd();
        (void)Bind(_local_addr);
    }

    /**
     * @brief Binds the socket to a local address.
     * @param local_addr Local address to bind to
     * @return true if binding succeeds, false otherwise
     */
    bool Bind(const InetAddr &local_addr) const {
        if(!CheckSockfd()) {
            return false;
        }
        if (!SocketUtils::Bind(_sockfd, local_addr)) {
            LOG_ERROR() << "bind to " << local_addr.GetIp() << ":" << local_addr.GetPort() << " failed";
            return false;
        }
        return true;
    }

    /**
     * @brief Sends data to a specific destination address.
     * @param data Data to send
     * @param peer_addr Destination address
     * @return Number of bytes sent, or false (0) on socket error
     */
    ssize_t SendTo(const std::string& data, const InetAddr& peer_addr) const {
        if(!CheckSockfd()) {
            return false;
        }
        return SocketUtils::SendTo(_sockfd, data, peer_addr);
    }

    /**
     * @brief Receives data and retrieves the sender's address.
     * @param sender_to_fill Output parameter to store sender's address
     * @return Received data as string, or empty string on error
     */
    std::string RecvFrom(InetAddr& sender_to_fill) const {
        if(!CheckSockfd()) {
            return {};
        }
        std::string recv_data;
        if (!SocketUtils::RecvFrom(_sockfd, recv_data, sender_to_fill)) {
            LOG_ERROR() << "recvfrom failed";
            return {};
        }
        return recv_data;
    }

    /**
     * @brief Receives data without retrieving sender address.
     * @return Received data as string, or empty string on error
     */
    std::string RecvFrom() const {
        if(!CheckSockfd()) {
            return {};
        }
        std::string recv_data;
        if (!SocketUtils::RecvFrom(_sockfd, recv_data)) {
            LOG_ERROR() << "recvfrom failed";
            return {};
        }
        return recv_data;
    }

private:
    int _sockfd;
    InetAddr _local_addr;
    InetAddr _peer_addr;
};
