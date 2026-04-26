// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <iostream>
// #include "log.hpp"
// using namespace sym;

// inline void Usage(const std::string &name) {
//     std::cerr << "Usage: " << name << " <ip> <port>" << std::endl;
// }

// int main(int argc, char* argv[]) {
//     if (argc != 3) {
//         Usage(argv[0]);
//         exit(1);
//     }

//     std::string server_ip = argv[1];
//     uint16_t server_port = std::stoi(argv[1]);
//     int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//     struct sockaddr_in server;
//     server.sin_family = AF_INET;
//     server.sin_port = htons(server_port);
//     if (inet_pton(AF_INET, server_ip.c_str(), &server.sin_addr) <= 0) {
//         LOG(log_level_t::FATAL) << "invalid IP address: " << server_ip;
//         exit(1);
//     }
//     return 0;
// }