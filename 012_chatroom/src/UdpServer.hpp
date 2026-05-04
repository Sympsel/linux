#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <functional>
#include <iostream>
#include <string>

#include "InetManager.hpp"
#include "Log.hpp"
#include "User.hpp"

// using callback_t = std::function<std::string(const std::string &)>;
using callback_t = std::function<void(const std::string &, const InetManager&, int sockfd)>;

using namespace sym;

// ReSharper disable once CppUnnamedNamespaceInHeaderFile
namespace {
    char op = '$';
};

class UdpServer {
public:
    UdpServer(callback_t cb, in_port_t port)
        : _sockfd(-1), _cb(cb), _port(port) {
    }

    ~UdpServer() {
        if (_sockfd >= 0) {
            close(_sockfd);
        }
    }

    void Init() {
        // 1. 创建 socket
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0); // IPPROTO_UDP
        if (_sockfd < 0) {
            LOG(log_level_t::FATAL) << "socket create error: " << strerror(errno);
            exit(1);
        }
        LOG(log_level_t::INFO) << "socket create successfully! Info[fd: " << _sockfd << "]";

        InetManager local{_port};
        local.Bind(_sockfd);
    }

    void Start() const {
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            InetManager peer;

            // 接收客户端消息(包括获取客户端地址及端口号)
            std::string inbuffer = peer.Recvfrom(_sockfd);
            // 构造响应消息
            if (_cb) {
                _cb(inbuffer, peer.InetAddr(), _sockfd);
            }
        }
    }

private:
    int _sockfd;
    std::string _ip;

    callback_t _cb;
    in_port_t _port;
};
