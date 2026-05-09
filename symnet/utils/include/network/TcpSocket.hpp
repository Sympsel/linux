#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <unistd.h>

#include "SocketUtils.h"
#include "../utils/Conf.hpp"

/**
 * @brief Base TCP socket class providing core TCP functionality.
 *
 * Manages the lifecycle of a TCP socket including creation,
 * connection management, and automatic cleanup.
 */
class TcpSocket {
private:
    /**
     * @brief Creates a new TCP socket file descriptor.
     * @return The created socket file descriptor
     * @note Terminates the program if socket creation fails
     */
    static int CreateSockfd() {
        const int sockfd = SocketUtils::CreateSocket(AF_INET,  SOCK_STREAM, 0);
        if (sockfd < 0) {
            LOG_FATAL() << "socket error";
            exit(2);
        }
        LOG_INFO() << "socket success. [fd: " << sockfd << "]";
        return sockfd;
    }
public:
    /**
     * @brief Puts a socket into listening state.
     * @param sockfd Socket file descriptor to listen on
     * @param backlog Maximum number of pending connections (default: 32)
     * @return true if listening starts successfully, false otherwise
     */
    static bool Listen(const int sockfd, const int backlog = 32) {
        if (const int ret = listen(sockfd, backlog); ret < 0) {
            LOG_ERROR() << "listen error";
            return false;
        }
        return true;
    }

    /**
     * @brief Accepts an incoming connection on a listening socket.
     * @param sockfd Server socket file descriptor
     * @param client_addr Output parameter to store client address information
     * @return Connected socket file descriptor for client communication, or -1 on error
     */
    [[nodiscard]] static int Accept(const int sockfd, InetAddr &client_addr) {
        sockaddr_in temp{};
        socklen_t addrlen = sizeof temp;

        const int conn_sockfd = accept(sockfd, reinterpret_cast<sockaddr *>(&temp), &addrlen);
        if (conn_sockfd < 0) {
            LOG_WARN() << "accept error";
            return -1;
        }
        client_addr.SetAddr(temp);
        LOG_INFO() << "accept success. [fd: " << conn_sockfd << "]";
        return conn_sockfd;
    }

    /**
     * @brief Initiates a connection to a remote address.
     * @param sockfd Socket file descriptor
     * @param peer Remote address to connect to
     * @return true if connection succeeds, false otherwise
     */
    static bool Connect(const int sockfd, const InetAddr &peer) {
        if (connect(sockfd, reinterpret_cast<const sockaddr *>(&peer.GetAddr()), peer.GetAddrLen()) < 0) {
            LOG_ERROR() << "connect error: " << strerror(errno);
            return false;
        }
        return true;
    }

    /**
     * @brief Constructs a TCP socket and creates the underlying file descriptor.
     */
    TcpSocket() {
        _sockfd = CreateSockfd();
    }

    /**
     * @brief Gets the socket file descriptor.
     * @return Reference to the socket file descriptor
     */
    [[nodiscard]] const int &GetSockfd() const {
        return _sockfd;
    }

    /**
     * @brief Destructor that automatically closes the socket if open.
     */
    ~TcpSocket() {
        if (_sockfd >= 0) {
            close(_sockfd);
            LOG_INFO() << "close socket. [fd: " << _sockfd << "]";
        }
    }

private:
    int _sockfd;
};
