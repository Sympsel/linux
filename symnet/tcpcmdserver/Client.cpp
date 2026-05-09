#include <iostream>
#include <unistd.h>

#include "src/TcpServer.hpp"


void Usage(const std::string &proc_name) {
    std::cout << "Usage: " << proc_name << " <ip> <port> [username]" << std::endl;
}

int main(const int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        Usage(argv[0]);
        exit(1);
    }

    const std::string server_ip = argv[1];
    const in_port_t server_port = std::stoi(argv[2]);
    std::string username;
    if (argc == 4) {
        username = argv[3];
    } else {
        username = "user";
    }

    const TcpCSocket client_socket;

    if (const InetAddr server_addr(server_port, server_ip); !client_socket.Connect(server_addr)) {
        LOG_ERROR() << "connect to " << server_ip << ":" << server_port << " failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    LOG_INFO() << "connected to server " << server_ip << ":" << server_port;

    const int sockfd = client_socket.GetSockfd();
    std::string cmd_str;

    while (true) {
        std::cout << username << "> ";
        std::cout.flush();

        if (std::getline(std::cin, cmd_str)) {
            if (const ssize_t n = SocketUtils::Write(client_socket.GetSockfd(), cmd_str); n < 0) {
                LOG_ERROR() << "send error: " << strerror(errno);
                break;
            }
            std::string ret_str;
            if (const bool read_status = SocketUtils::Read(client_socket.GetSockfd(), ret_str); !read_status) {
                LOG_ERROR() << "recv error: " << strerror(errno);
                break;
            }
            std::cout << ret_str << std::endl;
        } else {
            break;
        }
    }

    close(sockfd);
    return 0;
}
