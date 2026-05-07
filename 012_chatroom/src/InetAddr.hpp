#pragma once

#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <utility>

class InetAddr {
public:
    /**
     * @warning you should initialize it before use
     */
    InetAddr() : _port(), _addr() {
    }

    explicit InetAddr(const sockaddr_in& addr) : _port(), _addr(addr) {
        _port = ntohs(_addr.sin_port);
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &_addr.sin_addr, buffer, sizeof buffer);
        _ip = buffer;
    }

    InetAddr(const in_port_t& port, std::string ip) : _port(port), _ip(std::move(ip)), _addr() {
        _addr.sin_addr.s_addr = inet_addr(_ip.c_str());
        _addr.sin_port = htons(_port);
        _addr.sin_family = AF_INET;
    }

    InetAddr(InetAddr&&) = default;
    InetAddr(const InetAddr&) = default;
    InetAddr& operator=(InetAddr&&) = default;
    InetAddr& operator=(const InetAddr&) = default;

    [[nodiscard]] in_port_t GetPort() const {
        return _port;
    }

    [[nodiscard]] std::string GetIp() const {
        return _ip;
    }

    [[nodiscard]] sockaddr_in GetAddr() {
        return _addr;
    }

    [[nodiscard]] const sockaddr_in& GetAddr() const {
        return _addr;
    }

    void SetPort(const in_port_t& port) {
        _port = port;
        _addr.sin_port = htons(port);
    }

    void SetIp(std::string ip) {
        _addr.sin_addr.s_addr = inet_addr(ip.c_str());
        // move must be at last
        _ip = std::move(ip);
    }

    void SetAddr(const sockaddr_in& addr) {
        _addr = addr;
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &_addr.sin_addr, buffer, sizeof buffer);
        _ip = buffer;
        _port = ntohs(_addr.sin_port);
    }

    [[nodiscard]] socklen_t GetAddrLen() const {
        return sizeof _addr;
    }

    bool operator==(const InetAddr& other) const {
        return _port == other._port && _ip == other._ip;
    }

    ~InetAddr() = default;
private:
    in_port_t _port;
    std::string _ip;
    sockaddr_in _addr;
};
