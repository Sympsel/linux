#pragma once

#include "Socket.hpp"
#include "slog/Log.h"
#include <cstring>

class UdpSocket : public IUdpSocket {
public:
    explicit UdpSocket(const int sockfd = -1) {
        _sockfd = sockfd;
    }

    void CreateSocket() override {
        if ((_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            exit(CREATE_ERROR);
        }
    }

    void BindSocket(const in_port_t port) override {
        sockaddr_in _addr{};
        _addr.sin_addr.s_addr = htonl(INADDR_ANY);
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        if (const int ret = bind(_sockfd, reinterpret_cast<sockaddr *>(&_addr), sizeof _addr); ret == -1) {
            exit(BIND_ERROR);
        }
    }

    bool RecvFrom(std::string &str_to_fill, InetAddr &addr_to_fill) override {
        int max_len = 1024;
        char out_cstr[max_len];
        sockaddr_in temp{};
        socklen_t len = sizeof temp;

        const ssize_t n = recvfrom(_sockfd, out_cstr, sizeof out_cstr - 1, 0, reinterpret_cast<sockaddr *>(&temp),
                                   &len);
        if (n < 0) {
            LOG_ERROR() << "recvfrom error: " << strerror(errno);
            return false;
        }
        out_cstr[n] = '\0';
        str_to_fill += out_cstr;
        return true;
    }

    bool SendTo(const std::string &str, const InetAddr &peer) override {
        if (const ssize_t n = sendto(_sockfd, str.c_str(), str.size(),
            0, reinterpret_cast<const sockaddr *>(&peer.GetAddr()), peer.GetAddrLen()); n < 0) {
            LOG_ERROR() << "sendto error: " << strerror(errno);
            return false;
        }
        return true;
    }

    const int &GetSockfd() override {
        return _sockfd;
    }
};
