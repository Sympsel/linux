#pragma once

#include <cstring>

#include "../utils/InetAddr.hpp"
#include "../utils/Conf.hpp"

/**
 * @brief Utility class providing static methods for socket operations.
 *
 * This class provides a unified interface for common socket operations
 * including TCP and UDP communication. All methods are static and the
 * class cannot be instantiated.
 */
class SocketUtils {
public:
    SocketUtils() = delete;

    /**
     * @brief Creates a new socket with specified parameters.
     * @param domain Protocol family (e.g., AF_INET, AF_INET6)
     * @param type Socket type (e.g., SOCK_STREAM, SOCK_DGRAM)
     * @param protocol Protocol to use (default: 0 for default protocol)
     * @return File descriptor of the created socket, or -1 on error
     */
    static int CreateSocket(const int domain, const int type, const int protocol = 0) {
        const int sockfd = socket(domain, type, protocol);
        if (sockfd < 0) {
            return -1;
        }
        return sockfd;
    }

    /**
     * @brief Binds a socket to a specific address.
     * @param sockfd Socket file descriptor
     * @param peer Address to bind to
     * @return true if binding succeeds, false otherwise
     */
    static bool Bind(const int sockfd, const InetAddr &peer) {
        if (const int ret = bind(sockfd, reinterpret_cast<const sockaddr *>(&peer.GetAddr()),
                                 peer.GetAddrLen()); ret < 0) {
            return false;
        }
        return true;
    }

    /**
     * @brief Puts a socket into listening state.
     * @param sockfd Socket file descriptor
     * @param backlog Maximum number of pending connections
     * @return true if listening starts successfully, false otherwise
     */
    static bool Listen(const int sockfd, const int backlog) {
        return listen(sockfd, backlog) >= 0;
    }

    /**
     * @brief Accepts an incoming connection on a listening socket.
     * @param listen_sockfd Listening socket file descriptor
     * @param addr_client Output parameter to store client address
     * @return Connected socket file descriptor, or -1 on error
     */
    static int Accept(const int listen_sockfd, InetAddr &addr_client) {
        sockaddr_in temp{};
        socklen_t len = sizeof temp;
        const int connect_sockfd = accept(listen_sockfd, reinterpret_cast<sockaddr *>(&temp), &len);
        if (connect_sockfd < 0) {
            return -1;
        }
        addr_client.SetAddr(temp);
        return connect_sockfd;
    }

    /**
     * @brief Initiates a connection to a remote address.
     * @param sockfd Socket file descriptor
     * @param peer Remote address to connect to
     * @return true if connection succeeds, false otherwise
     */
    static bool Connect(const int sockfd, const InetAddr &peer) {
        return connect(sockfd, reinterpret_cast<const sockaddr *>(&peer.GetAddr()), peer.GetAddrLen()) >= 0;
    }

    /**
     * @brief Reads data from a socket.
     * @param sockfd Socket file descriptor
     * @return Received data as string, or empty string on error/connection close
     */
    static std::string Read(const int sockfd) {
        char inbuffer[Conf::network_buffer_length];
        const ssize_t n = read(sockfd, inbuffer, sizeof inbuffer - 1);
        if (n <= 0) {
            return {};
        }
        inbuffer[n] = '\0';
        return inbuffer;
    }

    /**
     * @brief Writes data to a socket.
     * @param sockfd Socket file descriptor
     * @param message Data to send
     * @return Number of bytes written, or negative value on error
     */
    static ssize_t Write(const int sockfd, const std::string &message) {
        return write(sockfd, message.c_str(), message.size());
    }

    /**
     * @brief Closes a socket.
     * @param sockfd Socket file descriptor to close
     * @return true if closed successfully or already invalid, false on error
     */
    static bool Close(const int sockfd) {
        if (sockfd >= 0) {
            if (const int ret = close(sockfd); ret < 0) {
                return false;
            }
            return true;
        }
        return true;
    }

    // === UDP Operations ===

    /**
     * @brief Sends data to a specific address via UDP.
     * @param sockfd UDP socket file descriptor
     * @param data Data to send
     * @param peer Destination address
     * @return Number of bytes sent, or -1 on error
     */
    static ssize_t SendTo(const int sockfd, const std::string &data, const InetAddr &peer) {
        const ssize_t n = sendto(
            sockfd, data.c_str(), data.size(), 0,
            reinterpret_cast<const sockaddr *>(&peer.GetAddr()), peer.GetAddrLen());
        if (n < 0) {
            LOG_ERROR() << "sendto error: " << strerror(errno);
            return -1;
        }
        LOG_DEBUG() << "send " << n << "bytes to " << peer.GetIp() << ":" << peer.GetPort();
        return n;
    }

    /**
     * @brief Receives data from a UDP socket and retrieves sender address.
     * @param sockfd UDP socket file descriptor
     * @param str_to_fill Output string to store received data
     * @param sender_to_fill Output parameter to store sender's address
     * @return true if reception succeeds, false on error
     */
    static bool RecvFrom(const int sockfd, std::string& str_to_fill, InetAddr &sender_to_fill) {
        char inbuffer[Conf::network_buffer_length];
        sockaddr_in temp{};
        socklen_t len = sizeof temp;
        if (const ssize_t n = recvfrom(sockfd, inbuffer, sizeof inbuffer - 1, 0,
                                       reinterpret_cast<sockaddr *>(&temp), &len); n < 0) {
            LOG_ERROR() << "recv error: " << strerror(errno);
            return false;
        } else {
            inbuffer[n] = '\0';
        }
        sender_to_fill.SetAddr(temp);
        str_to_fill = inbuffer;
        return true;
    }

    /**
     * @brief Receives data from a UDP socket without retrieving sender address.
     * @param sockfd UDP socket file descriptor
     * @param str_to_fill Output string to store received data
     * @return true if reception succeeds, false on error
     */
    static bool RecvFrom(const int sockfd, std::string& str_to_fill) {
        char inbuffer[Conf::network_buffer_length];
        if (const ssize_t n = recvfrom(sockfd, inbuffer, sizeof inbuffer, 0,
                                       nullptr, nullptr); n < 0) {
            LOG_ERROR() << "recv error: " << strerror(errno);
            return false;
        } else {
            inbuffer[n] = '\0';
        }
        str_to_fill = inbuffer;
        return true;
    }

    // === Configuration Options ===

    /**
     * @brief Enables address reuse option on a socket.
     * @param sockfd Socket file descriptor
     * @return true if option is set successfully, false otherwise
     */
    static bool SetReuseAddr(int sockfd);

    /**
     * @brief Enables port reuse option on a socket.
     * @param sockfd Socket file descriptor
     * @return true if option is set successfully, false otherwise
     */
    static bool SetReusePort(int sockfd);

    /**
     * @brief Sets a socket to non-blocking mode.
     * @param sockfd Socket file descriptor
     * @return true if mode is set successfully, false otherwise
     */
    static bool SetNonBlock(int sockfd);
};
