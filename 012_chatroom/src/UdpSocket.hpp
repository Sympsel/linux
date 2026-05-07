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

public:
    UdpSocket() = default;
    UdpSocket(UdpSocket &&) noexcept = default;
    explicit UdpSocket(const in_port_t &port, std::string ip = "0.0.0.0") : _addr_helper{port, std::move(ip)} {
    }
    explicit UdpSocket(const sockaddr_in &addr) : _addr_helper(addr) {
    }
    UdpSocket(const UdpSocket &) = default;
    UdpSocket &operator=(const UdpSocket &) = default;
    UdpSocket &operator=(UdpSocket &&) = default;

    // enable_log: we wish client won't receive log
    void Bind(const int sockfd, const bool enable_log = true) const {
        if (const int n = bind(sockfd, reinterpret_cast<const sockaddr *>(&_addr_helper.GetAddr()),
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

    [[nodiscard]] InetAddr GetInetAddr() const {
        return _addr_helper;
    }

    void SendTo(const int sockfd, const std::string &buffer) const {
        if (const ssize_t n = sendto(sockfd, buffer.c_str(), buffer.size(), 0,
                                     reinterpret_cast<const sockaddr *>(&_addr_helper.GetAddr()),
                                     _addr_helper.GetAddrLen()); n < 0) {
            LOG(log_level_t::ERROR) << "sendto error: " << strerror(errno);
        }
    }

    void SendWithUsername(const int sockfd, const std::string &buffer, const std::string &username) const {
        const std::string send_buffer = "[" + username + "]" + Conf::normal_msg_sign + " " + buffer;
        if (const ssize_t n = sendto(sockfd, send_buffer.c_str(), send_buffer.size(), 0,
                                     reinterpret_cast<const sockaddr *>(&_addr_helper.GetAddr()),
                                     _addr_helper.GetAddrLen()); n < 0) {
            LOG(log_level_t::ERROR) << "sendto error: " << strerror(errno);
        }
    }

    std::string Recvfrom(const int sockfd) {
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

    ~UdpSocket() = default;

private:
    InetAddr _addr_helper;
};
