#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include "log.hpp"
#include "src/UdpEchoServer.hpp"
using namespace sym;

inline void Usage(const std::string &name) {
    std::cerr << "Usage: " << name << " <port>" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        exit(1);
    }

    in_port_t server_port = std::stoi(argv[1]);

    UdpServer usvr(server_port);
    usvr.Init();
    usvr.Start();
    return 0;
}