#pragma once

#include <iostream>
#include <utility>

#include "UdpSocket.hpp"
#include "User.hpp"
#include "Thread.hpp"
#include "Config.hpp"

class ChatClient {
private:
    void RecvMsg() {
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            if (const std::string receive = _client_socket.Recvfrom(); !receive.empty()) {
                std::cout << receive << std::endl;
            }
        }
    }

    void SendMsg() const {
        std::string inbuffer;
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            std::getline(std::cin, inbuffer);
            if (!inbuffer.empty()) {
                _client_socket.SendWithUsername(inbuffer, _user.GetUsername(), _server_addr);
            }
        }
    }

public:
    explicit ChatClient(InetAddr server_addr,
                        const std::string &username = Conf::default_username) : _server_addr(std::move(server_addr)),
        _recv_thread([this] {
            RecvMsg();
        }), _send_thread([this] {
            SendMsg();
        }) {
        InitConf();

        // auto bind port and ip
        _client_socket.Bind(false);
        _user.SetUsername(username);
        _user.SetInetAddr(_client_socket.GetInetAddr());
    }


    void Start() {
        const std::string join_msg = "[" + Conf::system_name + "]" + Conf::system_msg_sign + " User " + _user.
                                     GetUsername() + " joined";
        _client_socket.SendToAddr(join_msg, _server_addr);
        _recv_thread.start();
        _send_thread.start();
        _recv_thread.join();
        _send_thread.join();
    }

    [[nodiscard]] const User &GetUser() const {
        return _user;
    }

    ~ChatClient() = default;

private:
    User _user;
    InetAddr _server_addr;
    UdpSocket _client_socket;
    Thread _recv_thread;
    Thread _send_thread;
};
