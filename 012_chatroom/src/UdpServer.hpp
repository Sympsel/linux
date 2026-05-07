#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstring>
#include <functional>
#include <string>
#include <utility>

#include "UdpSocket.hpp"
#include "Log.hpp"
#include "User.hpp"
#include "Config.hpp"

using callback_t = std::function<void(const std::string &, const User&, int sockfd)>;

using namespace sym;

class UdpServer {
public:
    UdpServer(callback_t cb, const in_port_t& port)
        : _sockfd(-1), _cb(std::move(cb)), _port(port) {
        InitConf();
    }

    ~UdpServer() {
        if (_sockfd >= 0) {
            close(_sockfd);
        }
    }

    void Init() {
        // 1. 创建 socket
        _sockfd = UdpSocket::Socket();
        if (_sockfd < 0) {
            LOG(log_level_t::FATAL) << "socket create error: " << strerror(errno);
            exit(1);
        }
        LOG(log_level_t::INFO) << "socket create successfully! Info[fd: " << _sockfd << "]";

        const UdpSocket local{_port};
        local.Bind(_sockfd);
    }

    void Start() const {
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            UdpSocket peer;

            // 接收客户端消息(包括获取客户端地址及端口号)
            std::string inbuffer = peer.Recvfrom(_sockfd);
            std::string username = Conf::default_username;
            // 构造响应消息
            if (_cb) {
                _cb(inbuffer, {peer, username}, _sockfd);
            }
        }
    }

private:
    int _sockfd;
    std::string _ip;

    callback_t _cb;
    in_port_t _port;
};
