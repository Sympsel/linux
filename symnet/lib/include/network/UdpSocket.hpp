#pragma once

#include "SocketUtils.hpp"
#include "../utils/InetAddr.hpp"
#include "../utils/Log.hpp"

#define CREATE_SOCKET (SocketUtils::CreateSocket(AF_INET, SOCK_DGRAM))

class UdpSocket {
private:
    bool CheckSockfd() const {
        if (_sockfd < 0) {
            LOG_ERROR() << "create UDP socket failed";
            return false;
        }
        return true;
    }

public:
    UdpSocket() : _sockfd(CREATE_SOCKET) {
        (void)CheckSockfd();
    }

    /**
     * @brief build and bind to specified address (for UDP server)
     * @param port for bind
     * @param ip for bind, set "0.0.0.0" to bind any client ip
     */
    explicit UdpSocket(const in_port_t &port, std::string ip = "0.0.0.0") : _sockfd(CREATE_SOCKET) {
        _local_addr = InetAddr(port, std::move(ip));
        (void)CheckSockfd();
        (void)Bind(_local_addr);
    }

    /**
     * @brief bind
     * @param local_addr local address
     * @return is_success
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

    ssize_t SendTo(const std::string& data, const InetAddr& peer_addr) const {
        if(!CheckSockfd()) {
            return false;
        }
        return SocketUtils::SendTo(_sockfd, data, peer_addr);
    }

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
