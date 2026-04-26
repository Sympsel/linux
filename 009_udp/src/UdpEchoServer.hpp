#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include "log.hpp"
#include <cstring>

using namespace sym;

class UdpEchoServer {
public:
    UdpEchoServer(const std::string& ip, in_port_t port)
        : _sockfd(-1), _ip(ip), _port(port) {}
    
    ~UdpEchoServer() {
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
        LOG(log_level_t::INFO) << "socket create successfully: " << _sockfd;
        
        // 2. 绑定地址和端口
        struct sockaddr_in local = {
            .sin_family = AF_INET,
            .sin_port = htons(_port),
            .sin_addr.s_addr = INADDR_ANY
        };
        
        // if (inet_pton(AF_INET, _ip.c_str(), &local.sin_addr) <= 0) {
        //     LOG(log_level_t::FATAL) << "invalid IP address: " << _ip;
        //     exit(1);
        // }
        
        int n = bind(_sockfd, (struct sockaddr*)&local, sizeof(local));
        if (n < 0) {
            LOG(log_level_t::FATAL) << "bind socket error: " << strerror(errno);
            exit(1);
        }
        LOG(log_level_t::INFO) << "bind success: " << _ip << ":" << _port;
    }

    void Start() {
        char inbuffer[1024];
        
        while (true) {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            
            // 接收客户端消息
            ssize_t n = recvfrom(_sockfd, inbuffer, sizeof(inbuffer) - 1, 0,
                                 (struct sockaddr*)&peer, &len);
            if (n < 0) {
                LOG(log_level_t::ERROR) << "recvfrom error: " << strerror(errno);
                continue;  // 继续服务其他请求
            }
            
            inbuffer[n] = '\0';
            
            // 转换客户端 IP 和端口为可读格式
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &peer.sin_addr, client_ip, sizeof(client_ip));
            
            LOG(log_level_t::INFO) << "get message: " << inbuffer \
                                   << ", from " << client_ip << ":" << ntohs(peer.sin_port);
            
            // 构造响应消息
            std::string echo_str = "server say: ";
            echo_str += inbuffer;
            
            // 发送响应
            int m = sendto(_sockfd, echo_str.c_str(), echo_str.size(), 0,
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
    in_port_t _port; // uint16_t
};
