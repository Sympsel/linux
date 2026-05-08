#pragma once

#include <functional>
#include <string>
#include <User.hpp>

class UdpSocket;

using UdpMsgCallback = std::function<void(
    const std::string &msg,
    const User &who,
    const UdpSocket &server_socket
)>;
