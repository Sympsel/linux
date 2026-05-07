#include <iostream>

#include "ChatClient.hpp"

using namespace sym;

namespace {
    std::string server_ip;
    in_port_t server_port;
}

inline void Usage(const std::string& name) {
    std::cerr << "Usage: " << name << " <server_ip> <server_port> [username]" << std::endl;
}

int main(const int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        Usage(argv[0]);
        exit(1);
    }

    std::string username;
    if (argc == 3) {
        username = Conf::default_username;
    } else {
        username = argv[3];
    }

    server_ip = argv[1];
    server_port = std::stoi(argv[2]);

    ChatClient client{UdpSocket{server_port, server_ip}, username};
    client.Start();
    return 0;
}