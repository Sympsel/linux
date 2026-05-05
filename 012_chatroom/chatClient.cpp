#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <iostream>

#include "ChatClient.hpp"
using namespace sym;

namespace {
    int sockfd;
    std::string server_ip;
    in_port_t server_port;
}

inline void Usage(const std::string& name) {
    std::cerr << "Usage: " << name << " <server_ip> <server_port>" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        Usage(argv[0]);
        exit(1);
    }

    server_ip = argv[1];
    server_port = std::stoi(argv[2]);

    ChatClient client{InetManager{server_port, server_ip}, "aaa"};

    // 1. 创建Udp socket
    InetManager local_addr(0);
    local_addr.Bind(sockfd, false);
    InetManager server_addr{server_port, server_ip};
    server_addr.SendTo(sockfd, "[SYSTEM] User joined");
    return 0;
}