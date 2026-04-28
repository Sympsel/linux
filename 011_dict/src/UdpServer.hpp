#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <functional>
#include <iostream>
#include <string>

#include "InetManager.hpp"
#include "log.hpp"

using callback_t = std::function<std::string(const std::string&)>;

using namespace sym;

namespace {
char op = '$';
};

class UdpServer {
   private:
    bool HandleOperation(const std::string& inbuffer) {
        std::istringstream iss(inbuffer);
        char _;
        iss >> _;
        std::string opera;
        iss >> opera;
        LOG(log_level_t::INFO) << "Get Op: " << opera;
        if (opera == "bye") {
            return true;
        } else {
            LOG(log_level_t::WARNING) << "Unknown operation!";
        }
        return false;
    }

   public:
    UdpServer(callback_t cb, in_port_t port)
        : _sockfd(-1), _cb(cb), _port(port) {}

    ~UdpServer() {
        if (_sockfd >= 0) {
            close(_sockfd);
        }
    }

    void Init() {
        // 1. 创建 socket
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // IPPROTO_UDP
        if (_sockfd < 0) {
            LOG(log_level_t::FATAL) << "socket create error: " << strerror(errno);
            exit(1);
        }
        LOG(log_level_t::INFO) << "socket create successfully! Info[fd: " << _sockfd << "]";

        InetManager local{_port};
        local.Bind(_sockfd);
    }

    void Start() {
        while (true) {
            InetManager peer;

            // 接收客户端消息(包括获取客户端地址及端口号)
            std::string inbuffer = peer.Recvfrom(_sockfd);
            if (inbuffer[0] == op) {
                if (HandleOperation(inbuffer)) {
                    LOG(log_level_t::INFO) << "Disconnect. [" << peer.Ip() << ':' << peer.Port() << ']';
                    peer.SendTo(_sockfd, "SIG_BYE");
                }
            } else {
                LOG(log_level_t::INFO) << "get a message: \"" << inbuffer << "\""
                                       << ", From[" << peer.Ip() << ":" << peer.Port() << ']';

                // 构造响应消息
                std::string result;
                if (_cb) {
                    result = _cb(inbuffer);
                }

                // 发送
                peer.SendTo(_sockfd, result);
            }
        }
    }

   private:
    int _sockfd;
    std::string _ip;

    callback_t _cb;
    in_port_t _port;  // uint16_t
};
