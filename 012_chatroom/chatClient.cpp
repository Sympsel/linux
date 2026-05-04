#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <iostream>

#include "InetManager.hpp"
#include "Log.hpp"
#include "Thread.hpp"
using namespace sym;

namespace {
    int sockfd;
    std::string server_ip;
    in_port_t server_port;
}

void RecvMsg() {
    while (true) {
        InetManager peer;
        if (const std::string receive = peer.Recvfrom(sockfd); !receive.empty()) {
            std::cout << "Translated=> " << receive << std::endl;
        }
    }
}

void SendMsg() {
    InetManager server_addr{server_port, server_ip};
    std::string inbuffer;
    while (true) {
        InetManager tmp;
        std::cout << "Send a Word# ";
        std::getline(std::cin, inbuffer);
        if (!inbuffer.empty()) {
            server_addr.SendTo(sockfd, inbuffer);
        }
    }
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

    // 1. 创建Udp socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOG(log_level_t::FATAL) << "socket error";
        exit(1);
    }

    Thread recver(RecvMsg);
    Thread sender(SendMsg);
    recver.start();
    sender.start();
    recver.join();
    sender.join();
    return 0;
}