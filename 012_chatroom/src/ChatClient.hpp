#pragma once

#include <iostream>
#include <utility>

#include "InetManager.hpp"
#include "User.hpp"
#include "Thread.hpp"

inline std::string default_username = "unnamed";

class ChatClient {
private:
    void RecvMsg() const {
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
                server_addr.SendWithUsername(_sockfd, inbuffer, _user.GetUsername());
            }
        }
    }

public:
    explicit ChatClient(InetManager server_addr, const std::string &username = default_username) : _sockfd(-1),
        server_addr(std::move(server_addr)),
        _recv_thread([this] {
            RecvMsg();
        }), _send_thread([this] {
            SendMsg();
        }) {
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sockfd < 0) {
            std::cerr << "socket error" << std::endl;
            exit(1);
        }
        // auto bind port and ip
        InetManager local(0);
        local.Bind(_sockfd, false);
        _user.SetUsername(username);
        _user.SetInetManager(local);
    }


    void Start() {
        const std::string join_msg = "[SYSTEM]$ User " + _user.GetUsername() + " joined";
        server_addr.SendTo(_sockfd, join_msg);
        _recv_thread.start();
        _send_thread.start();
        _recv_thread.join();
        _recv_thread.join();
    }

    [[nodiscard]] const User &GetUser() const {
        return _user;
    }

    ~ChatClient() = default;

private:
    int _sockfd;
    User _user;
    InetManager server_addr;
    Thread _recv_thread;
    Thread _send_thread;
};
