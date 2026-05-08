#pragma once

#include "SocketUtils.hpp"
#include "TcpSocket.hpp"

class TcpCSocket {
public:
    explicit TcpCSocket() = default;

    [[nodiscard]] bool Connect(const InetAddr& peer) const {
        return SocketUtils::Connect(_socket.GetSockfd(), peer);
    }

    [[nodiscard]] bool Connect(InetAddr&& peer) const {
        return SocketUtils::Connect(_socket.GetSockfd(), peer);
    }

    [[nodiscard]] const int &GetSockfd() const {
        return _socket.GetSockfd();
    }

    ~TcpCSocket() = default;
private:
    TcpSocket _socket;
};
