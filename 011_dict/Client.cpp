#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <iostream>

#include "InetManager.hpp"
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
    InetManager target{server_port, server_ip};
    std::string inbuffer;

    while (true) {
        InetManager tmp;
        std::cout << "Send a Word# ";
        std::getline(std::cin, inbuffer);
        if (inbuffer.empty()) continue;

        // sent require to udp-server
        target.SendTo(sockfd, inbuffer);

        // peer 只用来接收，获取服务器响应
        InetManager peer;
        std::string receive = peer.Recvfrom(sockfd);
        if (receive.empty()) continue;
        if (receive == "SIG_BYE") {
            std::cout << "Trans Server: Bye.";
            break;
        }
        std::cout << "Translated=> " << receive << std::endl;
    }

    return 0;
}