#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

#include "log.hpp"
using namespace sym;

class InetManager {
   private:
    std::string IpToString(const struct in_addr& sin_addr) {
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin_addr, buffer, sizeof buffer);
        return buffer;
    }
   public:
    InetManager() {}
    InetManager(in_port_t port, const std::string& ip = "0.0.0.0") : _port(port), _ip(ip) {
        _addr.sin_family = AF_INET;
        _addr.sin_addr.s_addr = inet_addr(_ip.c_str());
        _addr.sin_port = htons(_port);
    }
    InetManager(const struct sockaddr_in& addr) : _addr(addr) {
        _port = ntohs(_addr.sin_port);
        _ip = IpToString(_addr.sin_addr);
    }

    std::string Ip() {
        return _ip;
    }

    in_port_t Port() {
        return _port;
    }

    void Bind(int sockfd) {
        int n = bind(sockfd, (struct sockaddr*)&_addr, Len());
        if (n < 0) {
            LOG(log_level_t::FATAL) << "bind socket error: " << strerror(errno);
            exit(1);
        }
        LOG(log_level_t::INFO) << "bind success! Info[" << _ip << ":" << _port << "]";
    }

    struct sockaddr_in InetAddr() {
        return _addr;
    }

    socklen_t Len() {
        return sizeof _addr;
    }

    void SendTo(int sockfd, const std::string& buffer) {
        ssize_t n = sendto(sockfd, buffer.c_str(), buffer.size(), 0, (struct sockaddr*)&_addr, Len());
        if (n < 0) {
            LOG(log_level_t::ERROR) << "sendto error: " << strerror(errno);
        }
    }

    std::string Recvfrom(int sockfd) {
        char buffer[1024];
        socklen_t len = Len();
        // len是输入/输出参数,需要传入缓冲区大小,输出实际大小
        ssize_t n = recvfrom(sockfd, buffer, sizeof buffer - 1, 0, (struct sockaddr*)&_addr, &len);
        if (n < 0) {
            LOG(log_level_t::ERROR) << "recvfrom error: " << strerror(errno);
            return "";
        }
        buffer[n] = '\0';

        // 当接收完数据后,需要更新类内的_port, _ip
        _port = ntohs(_addr.sin_port);
        _ip = IpToString(_addr.sin_addr);
        return buffer;
    }

    ~InetManager() {}

   private:
    in_port_t _port;
    std::string _ip;

    struct sockaddr_in _addr;
};