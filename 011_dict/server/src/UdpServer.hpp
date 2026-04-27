#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <iostream>
#include <string>
#include <functional>
#include "log.hpp"

using callback_t = std::function<std::string(const std::string&)>;

using namespace sym;

class UdpServer {
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
        LOG(log_level_t::INFO) << "socket create successfully! Info: [fd: " << _sockfd << "]";

        struct sockaddr_in local;
        memset(&local, 0, sizeof local);
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = INADDR_ANY;

        int n = bind(_sockfd, (struct sockaddr*)&local, sizeof(local));
        if (n < 0) {
            LOG(log_level_t::FATAL) << "bind socket error: " << strerror(errno);
            exit(1);
        }
        LOG(log_level_t::INFO) << "bind success! Info[" << _ip << ":" << _port << "]";
    }

    void Start() {
        char inbuffer[1024];

        while (true) {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);

            // 接收客户端消息
            // len是输入/输出参数,需要传入缓冲区大小,输出实际大小
            memset(inbuffer, 0, sizeof inbuffer);
            ssize_t n = recvfrom(_sockfd, inbuffer, sizeof(inbuffer) - 1, 0,
                                 (struct sockaddr*)&peer, &len);
            if (n < 0) {
                LOG(log_level_t::ERROR) << "recvfrom error: " << strerror(errno);
                continue;
            }
            inbuffer[n] = '\0';

            in_port_t client_port = ntohs(peer.sin_port);
            std::string client_ip = inet_ntoa(peer.sin_addr);


            LOG(log_level_t::INFO) << "get a message: \"" << inbuffer << "\""
                                   << ", from " << client_ip << ":" << client_port;

            // 构造响应消息
            std::string result;
            if (_cb) {
                result = _cb(inbuffer);
            }

            // 发送
            int m = sendto(_sockfd, result.c_str(), result.size(), 0,
                           (struct sockaddr*)&peer, len);
            if (m < 0) {
                LOG(log_level_t::ERROR) << "sendto error: " << strerror(errno);
            }
            inbuffer[m] = '\0';
        }
    }

   private:
    int _sockfd;
    std::string _ip;

    callback_t _cb;
    in_port_t _port;  // uint16_t
};
