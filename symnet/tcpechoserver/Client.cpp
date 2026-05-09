#include <iostream>
#include <unistd.h>
#include "src/TcpEchoServer.hpp"


void Usage(const std::string &proc_name) {
    std::cout << "Usage: " << proc_name << " <ip> <port>" << std::endl;
}

int main(const int argc, char *argv[]) {
    if (argc != 3) {
        Usage(argv[0]);
        exit(1);
    }

    const std::string server_ip = argv[1];
    const in_port_t server_port = std::stoi(argv[2]);

    const TcpCSocket client_socket;

    if (const InetAddr server_addr(server_port, server_ip); !client_socket.Connect(server_addr)) {
        std::cerr << "connect to " << server_ip << ":" << server_port << " failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    LOG_INFO() << "connected to server " << server_ip << ":" << server_port;

    const int sockfd = client_socket.GetSockfd();
    char send_buffer[default_buffer_length];
    char recv_buffer[default_buffer_length];

    while (true) {
        std::cout << "> ";
        std::cout.flush();

        if (std::cin.getline(send_buffer, sizeof send_buffer)) {
            if (const ssize_t n = write(sockfd, send_buffer, strlen(send_buffer)); n < 0) {
                LOG_ERROR() << "send error: " << strerror(errno);
                break;
            }

            if (const ssize_t m = read(sockfd, recv_buffer, sizeof recv_buffer - 1); m > 0) {
                recv_buffer[m] = '\0';
                std::cout << "Server: " << recv_buffer << std::endl;
            } else if (m == 0) {
                LOG_INFO() << "server closed connection";
                break;
            } else {
                LOG_ERROR() << "recv error: " << strerror(errno);
                break;
            }
        } else {
            break;
        }
    }

    close(sockfd);
    return 0;
}
