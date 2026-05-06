#include <iostream>

#include "src/TcpEchoServer.hpp"

void Usage(const std::string& procname) {
    std::cout << "Usage: " << procname << " <port>" << std::endl;
}

int main(const int argc, char* argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        exit(1);
    }
    in_port_t server_port = std::stoi(argv[1]);
    const auto tcp_echo_server = std::make_unique<TcpEchoServer>(server_port);
    tcp_echo_server->Init();
    tcp_echo_server->Start();
    return 0;
}