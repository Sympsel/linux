#pragma once

#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>
#include <utility>

#include "Config.hpp"
#include "InetAddr.hpp"
#include "Log.hpp"
using namespace sym;

class UdpSocket {
private:
    static std::string IpToString(const in_addr &sin_addr) {
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin_addr, buffer, sizeof buffer);
        return buffer;
    }

    static int Socket() {
        const int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // IPPROTO_UDP
        if (sockfd < 0) {
            LOG_ERROR() << "socket error: " << strerror(errno);
            return -1;
        }
        return sockfd;
    }

public:
    UdpSocket() : _sockfd(Socket()) {
    }

    explicit UdpSocket(const in_port_t &port, std::string ip = "0.0.0.0") : _sockfd(Socket()),
                                                                   _addr_helper{port, std::move(ip)} {
    }

    explicit UdpSocket(const sockaddr_in &addr) : _sockfd(Socket()), _addr_helper(addr) {
    }

    explicit UdpSocket(InetAddr addr_helper) : _sockfd(Socket()), _addr_helper(std::move(addr_helper)) {
    }

    UdpSocket(const UdpSocket &) = default;

    UdpSocket(UdpSocket &&other) noexcept : _sockfd(other._sockfd), _addr_helper(std::move(other._addr_helper)) {
        // make original socket invalid to ensure socket is hold by only one object
        other._sockfd = -1;
    }

    UdpSocket &operator=(const UdpSocket &) = default;

    UdpSocket &operator=(UdpSocket &&other) noexcept {
        // we should release the resource before assigning
        if (this != &other) {
            if (_sockfd >= 0) {
                close(_sockfd);
            }
            _sockfd = other._sockfd;
            _addr_helper = std::move(other._addr_helper);
            other._sockfd = -1;
        }
        return *this;
    }

    void Bind(const bool enable_log = true) const {
        if (const int n = bind(_sockfd, reinterpret_cast<const sockaddr *>(&_addr_helper.GetAddr()),
                               _addr_helper.GetAddrLen()); n < 0) {
            if (enable_log) {
                LOG(log_level_t::FATAL) << "bind socket error: " << strerror(errno);
            }
            exit(1);
        }
        if (enable_log) {
            LOG(log_level_t::INFO) << "bind socket success";
        }
    }

    [[nodiscard]] const InetAddr &GetInetAddr() const {
        return _addr_helper;
    }

    [[nodiscard]] const int &GetSockfd() const {
        return _sockfd;
    }

    void SendTo(const std::string &buffer) const {
        if (const ssize_t n = sendto(_sockfd, buffer.c_str(), buffer.size(), 0,
                                     reinterpret_cast<const sockaddr *>(&_addr_helper.GetAddr()),
                                     _addr_helper.GetAddrLen()); n < 0) {
            LOG_ERROR() << "sendto error: " << strerror(errno);
        }
    }

    void SendWithUsernameTo(const std::string &buffer, const std::string &username) const {
        const std::string send_buffer = "[" + username + "]" + Conf::normal_msg_sign + " " + buffer;
        if (const ssize_t n = sendto(_sockfd, send_buffer.c_str(), send_buffer.size(), 0,
                                     reinterpret_cast<const sockaddr *>(&_addr_helper.GetAddr()),
                                     _addr_helper.GetAddrLen()); n < 0) {
            LOG_ERROR() << "sendto error: " << strerror(errno);
        }
    }

    void SendToAddr(const std::string &buffer, const InetAddr& addr) const {
        if (const ssize_t n = sendto(_sockfd, buffer.c_str(), buffer.size(), 0,
                                     reinterpret_cast<const sockaddr *>(&addr.GetAddr()),
                                     addr.GetAddrLen()); n < 0) {
            LOG_ERROR() << "sendto error: " << strerror(errno);
                                     }
    }

    void SendWithUsername(const std::string &buffer, const std::string &username, const InetAddr& addr) const {
        const std::string send_buffer = "[" + username + "]" + Conf::normal_msg_sign + " " + buffer;
        if (const ssize_t n = sendto(_sockfd, send_buffer.c_str(), send_buffer.size(), 0,
                                     reinterpret_cast<const sockaddr *>(&addr.GetAddr()),
                                     addr.GetAddrLen()); n < 0) {
            LOG_ERROR() << "sendto error: " << strerror(errno);
                                     }
    }

    std::string Recvfrom() {
        return RecvfromSockfd(_sockfd);
    }

    std::string RecvfromSockfd(const int sockfd) {
        char buffer[1024];
        sockaddr_in temp{};
        // len是输入/输出参数,需要传入缓冲区大小,输出实际大小
        socklen_t len = sizeof temp;
        const ssize_t n = recvfrom(sockfd, buffer, sizeof buffer - 1, 0, reinterpret_cast<sockaddr *>(&temp), &len);
        if (n < 0) {
            LOG(log_level_t::ERROR) << "recvfrom error: " << strerror(errno);
            return {};
        }
        buffer[n] = '\0';
        _addr_helper.SetAddr(temp);
        return buffer;
    }

    ~UdpSocket() {
        if (_sockfd >= 0) {
            close(_sockfd);
        }
    }

private:
    int _sockfd;
    InetAddr _addr_helper;
};
