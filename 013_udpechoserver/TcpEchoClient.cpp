#include "TcpSocket.hpp"

void Usage(const std::string& procname) {
    std::cout << "Usage: " << procname << "<ip> <port>" << std::endl;
}

int main(const int argc, char* argv[]) {
    if (argc != 3) {
        Usage(argv[0]);
        exit(1);
    }
    const std::string server_ip = argv[1];
    const in_port_t server_port = static_cast<int>(std::stol(argv[2]));
    const TcpSocket client_socket;
    const InetAddr server_addr{server_port, server_ip};
    if (const bool connect_status = client_socket.Connect(server_addr); !connect_status) {
        // handle reconnect or exit
        exit(1);
    }
    return 0;
}
