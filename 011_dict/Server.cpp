#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <iostream>

#include "log.hpp"
#include "src/DictServer.hpp"
#include "src/UdpServer.hpp"
using namespace sym;

inline void Usage(const std::string& name) {
    std::cerr << "Usage: " << name << " <port>" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        exit(1);
    }

    in_port_t udp_server_port = std::stoi(argv[1]);
    std::unique_ptr<DictServer> dictServer = std::make_unique<DictServer>();
    std::unique_ptr<UdpServer> udpServer = std::make_unique<UdpServer>([&dictServer](const std::string& word) -> std::string {
        return dictServer->Translate(word);
    }, udp_server_port);
    udpServer->Init();
    udpServer->Start();

    return 0;
}