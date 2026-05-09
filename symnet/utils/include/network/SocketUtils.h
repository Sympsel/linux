#pragma once

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
    static int CreateSocket(int domain, int type, int protocol = 0);

    /**
     * @brief Binds a socket to a specific address.
     * @param sockfd Socket file descriptor
     * @param peer Address to bind to
     * @return true if binding succeeds, false otherwise
     */
    static bool Bind(int sockfd, const InetAddr &peer);

    /**
     * @brief Puts a socket into listening state.
     * @param sockfd Socket file descriptor
     * @param backlog Maximum number of pending connections
     * @return true if listening starts successfully, false otherwise
     */
    static bool Listen(int sockfd, int backlog);

    /**
     * @brief Accepts an incoming connection on a listening socket.
     * @param listen_sockfd Listening socket file descriptor
     * @param addr_client Output parameter to store client address
     * @return Connected socket file descriptor, or -1 on error
     */
    static int Accept(int listen_sockfd, InetAddr &addr_client);

    /**
     * @brief Initiates a connection to a remote address.
     * @param sockfd Socket file descriptor
     * @param peer Remote address to connect to
     * @return true if connection succeeds, false otherwise
     */
    static bool Connect(int sockfd, const InetAddr &peer);

    /**
     * @brief Reads data from a socket.
     * @param sockfd Socket file descriptor
     * @param max_size Maximum number of bytes to read, auto set by Conf::network_buffer_length
     * @return Received data as string, or empty string on error/connection close
     */
    static std::string Read(int sockfd, size_t max_size = Conf::network_buffer_length);

    /**
     * @brief Reads data from a socket into a string.
     * @param sockfd Socket file descriptor
     * @param str_to_fill Output string to store received data
     * @param max_size Maximum number of bytes to read, auto set by Conf::network_buffer_length
     * @return true if reception succeeds, false on error
     */
    static bool Read(int sockfd, std::string& str_to_fill, size_t max_size = Conf::network_buffer_length);

    /**
     * @brief Writes data to a socket.
     * @param sockfd Socket file descriptor
     * @param message Data to send
     * @return Number of bytes written, or negative value on error
     */
    static ssize_t Write(int sockfd, const std::string &message);

    /**
     * @brief Closes a socket.
     * @param sockfd Socket file descriptor to close
     * @return true if closed successfully or already invalid, false on error
     */
    static bool Close(int sockfd);

    // === UDP Operations ===

    /**
     * @brief Sends data to a specific address via UDP.
     * @param sockfd UDP socket file descriptor
     * @param data Data to send
     * @param peer Destination address
     * @return Number of bytes sent, or -1 on error
     */
    static ssize_t SendTo(int sockfd, const std::string &data, const InetAddr &peer);

    /**
     * @brief Receives data from a UDP socket and retrieves sender address.
     * @param sockfd UDP socket file descriptor
     * @param str_to_fill Output string to store received data
     * @param sender_to_fill Output parameter to store sender's address
     * @return true if reception succeeds, false on error
     */
    static bool RecvFrom(int sockfd, std::string& str_to_fill, InetAddr &sender_to_fill);

    /**
     * @brief Receives data from a UDP socket without retrieving sender address.
     * @param sockfd UDP socket file descriptor
     * @param str_to_fill Output string to store received data
     * @return true if reception succeeds, false on error
     */
    static bool RecvFrom(int sockfd, std::string& str_to_fill);

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
