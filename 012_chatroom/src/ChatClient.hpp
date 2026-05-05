#pragma once

#include <iostream>
#include <utility>

#include "InetManager.hpp"
#include "User.hpp"
#include "Thread.hpp"

class ChatClient {
private:

    void RecvMsg() {
        InetManager peer;
        while (true) {
            if (const std::string receive = peer.Recvfrom(_sockfd); !receive.empty()) {
                std::cout << receive << std::endl;
            }
        }
    }

    void SendMsg() {
        std::string inbuffer;
        while (true) {
            std::getline(std::cin, inbuffer);
            if (!inbuffer.empty()) {
                server_addr.SendTo(_sockfd, inbuffer);
            }
        }
    }
public:
    ChatClient(InetManager  server_addr, const std::string& username = "unnamed") : _sockfd(-1), _user(User{}), server_addr(std::move(server_addr)),
    _recv_thread([this]() {
        RecvMsg();
    }), _send_thread([this]() {
        SendMsg();
    }) {
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sockfd < 0) {
            std::cerr << "socket error" << std::endl;
            exit(1);
        }
        InetManager local;
        local.Bind(_sockfd, false);
        _user = User(local, username);
    }


    void Start() {

    }

private:
    int _sockfd;
    User _user;
    InetManager server_addr;
    Thread _recv_thread;
    Thread _send_thread;
};

