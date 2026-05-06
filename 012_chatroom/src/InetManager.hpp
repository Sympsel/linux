#pragma once

#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>
#include <utility>

#include "Config.hpp"
#include "Log.hpp"
using namespace sym;

class InetManager {
   private:
    static std::string IpToString(const in_addr& sin_addr) {
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin_addr, buffer, sizeof buffer);
        return buffer;
    }
   public:
    InetManager() : _port{}, _addr{} {}
    InetManager(InetManager&&) noexcept = default;

    explicit InetManager(const in_port_t& port, std::string ip = "0.0.0.0") : _port{port}, _ip{std::move(ip)}, _addr{} {
        _addr.sin_family = AF_INET;
        _addr.sin_addr.s_addr = inet_addr(_ip.c_str());
        _addr.sin_port = htons(_port);
    }

    explicit InetManager(const sockaddr_in& addr) : _port{}, _addr(addr) {
        _port = ntohs(_addr.sin_port);
        _ip = IpToString(_addr.sin_addr);
    }

    InetManager(const InetManager&) = default;
    InetManager& operator=(const InetManager&) = default;
    InetManager& operator=(InetManager&&) = default;

    [[nodiscard]] std::string GetIp() const {
        return _ip;
    }

    [[nodiscard]] in_port_t GetPort() const {
        return _port;
    }

    // enable_log: we wish client won't receive log
    void Bind(const int sockfd, const bool enable_log = true) {
        if (const int n = bind(sockfd, reinterpret_cast<sockaddr *>(&_addr), Len()); n < 0) {
            if (enable_log) {
                LOG(log_level_t::FATAL) << "bind socket error: " << strerror(errno);
            }
            exit(1);
        }
        if (enable_log) {
            LOG(log_level_t::INFO) << "bind socket success";
        }
    }

    [[nodiscard]] sockaddr_in InetAddr() const {
        return _addr;
    }

    [[nodiscard]] socklen_t Len() const {
        return sizeof _addr;
    }

    void SendTo(const int sockfd, const std::string& buffer) {
        if (const ssize_t n = sendto(sockfd, buffer.c_str(), buffer.size(), 0, reinterpret_cast<sockaddr *>(&_addr), Len()); n < 0) {
            LOG(log_level_t::ERROR) << "sendto error: " << strerror(errno);
        }
    }

    void SendWithUsername(const int sockfd, const std::string& buffer, const std::string &username) {
        const std::string send_buffer = "[" +  username + "]" + Conf::normal_msg_sign + " " + buffer;
        if (const ssize_t n = sendto(sockfd, send_buffer.c_str(), send_buffer.size(), 0, reinterpret_cast<sockaddr *>(&_addr), Len()); n < 0) {
            LOG(log_level_t::ERROR) << "sendto error: " << strerror(errno);
        }
    }

    std::string Recvfrom(const int sockfd) {
        char buffer[1024];
        socklen_t len = Len();
        // len是输入/输出参数,需要传入缓冲区大小,输出实际大小
        const ssize_t n = recvfrom(sockfd, buffer, sizeof buffer - 1, 0, reinterpret_cast<sockaddr *>(&_addr), &len);
        if (n < 0) {
            LOG(log_level_t::ERROR) << "recvfrom error: " << strerror(errno);
            return {};
        }
        buffer[n] = '\0';

        _port = ntohs(_addr.sin_port);
        _ip = IpToString(_addr.sin_addr);
        return buffer;
    }

    bool operator==(const InetManager& o) const {
        return _port == o._port && _ip == o._ip;
    }

    ~InetManager() = default;

   private:
    in_port_t _port;
    std::string _ip;

    sockaddr_in _addr;
};