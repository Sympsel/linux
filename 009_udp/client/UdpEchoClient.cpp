#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <iostream>

#include "log.hpp"
using namespace sym;

inline void Usage(const std::string& name) {
    std::cerr << "Usage: " << name << " <server_ip> <server_port>" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        Usage(argv[0]);
        exit(1);
    }

    std::string server_ip = argv[1];
    in_port_t server_port = std::stoi(argv[2]);

    // 1. 创建Udp socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOG(log_level_t::FATAL) << "socket error";
        exit(1);
    }

    // 客户端不能自己显式bind端口号, 由操作系统自行bind
    // 2. 构建目标服务器信息
    struct sockaddr_in server;
    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());
    std::string inbuffer;

    while (true) {
        std::cout << "Send Message# ";
        std::cin >> inbuffer;
        ssize_t n = sendto(sockfd, inbuffer.c_str(), inbuffer.size(), 0,
                           (struct sockaddr*)&server, sizeof server);
        if (n < 0) {
            LOG(log_level_t::ERROR) << "send message error";
            continue;
        }
        struct sockaddr_in tmp;
        socklen_t len = sizeof tmp;
        char buffer[1024];
        ssize_t m = recvfrom(sockfd, buffer, sizeof buffer - 1, 0, (struct sockaddr*)&tmp, &len);
        if (m < 0) {
            LOG(log_level_t::ERROR) << "recvfrom error: " << strerror(errno);
            continue;
        }

        if (inbuffer == "bye") break;
    }

    return 0;
}