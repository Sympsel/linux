#include <iostream>
#include "TcpEchoServer.hpp"

void Usage(const std::string& proc_name) {
    std::cout << "Usage: " << proc_name << " [port]" << std::endl;
}

// for demo
int main(const int argc, char* argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        exit(1);
    }
    in_port_t server_port = std::stoi(argv[1]);
    const auto server = std::make_unique<TcpEchoServer>(server_port);
    server->Init();
    server->Start();
    return 0;
}