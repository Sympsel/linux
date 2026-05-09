#pragma once

#include "SocketUtils.h"
#include "TcpSocket.hpp"

/**
 * @brief TCP client socket wrapper class.
 *
 * Provides a simplified interface for TCP client operations,
 * encapsulating connection establishment and socket management.
 */
class TcpCSocket {
public:
    explicit TcpCSocket() = default;

    /**
     * @brief Establishes a connection to a remote server.
     * @param peer Server address to connect to
     * @return true if connection succeeds, false otherwise
     */
    [[nodiscard]] bool Connect(const InetAddr& peer) const {
        return SocketUtils::Connect(_socket.GetSockfd(), peer);
    }

    /**
     * @brief Establishes a connection to a remote server (rvalue overload).
     * @param peer Server address to connect to (moved)
     * @return true if connection succeeds, false otherwise
     */
    [[nodiscard]] bool Connect(InetAddr&& peer) const {
        return SocketUtils::Connect(_socket.GetSockfd(), peer);
    }

    /**
     * @brief Gets the underlying socket file descriptor.
     * @return Reference to the socket file descriptor
     */
    [[nodiscard]] const int &GetSockfd() const {
        return _socket.GetSockfd();
    }

    ~TcpCSocket() = default;
private:
    TcpSocket _socket;
};
