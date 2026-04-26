#include <iostream>
#include "src/UdpEchoServer.hpp"

inline void Usage(const std::string &name) {
    std::cerr << "Usage: " << name << " <ip> <port>" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        Usage(argv[0]);
        exit(1);
    }
    std::string server_ip = argv[1];
    in_port_t server_port = std::stoi(argv[2]);
    UdpEchoServer uesvr(server_ip, server_port);
    uesvr.Init();
    uesvr.Start();
    return 0;
}

