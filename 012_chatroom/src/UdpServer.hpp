#pragma once

#include <netinet/in.h>

#include <functional>
#include <string>
#include <utility>

#include "UdpSocket.hpp"
#include "Log.hpp"
#include "User.hpp"
#include "Config.hpp"

using callback_t = std::function<void(const std::string &, const User &, const UdpSocket&)>;

using namespace sym;

class UdpServer {
public:
    UdpServer(callback_t cb, const in_port_t &port)
        : _cb(std::move(cb)), _port(port) {
        InitConf();
    }

    ~UdpServer() = default;

    void Init() {
        // 1. 创建 socket
        _server_sock = UdpSocket{_port};
        _server_sock.Bind();
    }

    void Start() const {
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            UdpSocket peer;
            // receive message (port and ip are included)
            std::string inbuffer = peer.RecvfromSockfd(_server_sock.GetSockfd());
            // build response message
            if (_cb) {
                _cb(inbuffer, {peer.GetInetAddr(), Conf::default_username}, _server_sock);
            }
        }
    }

private:
    UdpSocket _server_sock;
    std::string _ip;

    callback_t _cb;
    in_port_t _port;
};
