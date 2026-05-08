#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <utility>

/**
 * @brief Wrapper class for IPv4 socket addresses.
 *
 * Simplifies working with sockaddr_in structures by providing
 * convenient getters/setters and automatic conversion between
 * binary and string representations of IP addresses.
 */
class InetAddr {
public:
    /**
     * @brief Default constructor. Initializes to zero values.
     * @warning Must be properly initialized before use via SetAddr(), SetIp(), or SetPort().
     */
    InetAddr() : _port(), _addr() {
    }

    /**
     * @brief Constructs from an existing sockaddr_in structure.
     * @param addr Socket address structure to copy
     */
    explicit InetAddr(const sockaddr_in& addr) : _port(), _addr(addr) {
        _port = ntohs(_addr.sin_port);
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &_addr.sin_addr, buffer, sizeof buffer);
        _ip = buffer;
    }

    /**
     * @brief Constructs from port and IP address components.
     * @param port Port number in host byte order
     * @param ip IP address as string (e.g., "192.168.1.1")
     */
    InetAddr(const in_port_t& port, std::string ip) : _port(port), _ip(std::move(ip)), _addr() {
        _addr.sin_addr.s_addr = inet_addr(_ip.c_str());
        _addr.sin_port = htons(_port);
        _addr.sin_family = AF_INET;
    }

    /**
     * @brief Gets the port number.
     * @return Port number in host byte order
     */
    [[nodiscard]] in_port_t GetPort() const {
        return _port;
    }

    /**
     * @brief Gets the IP address as a string.
     * @return IP address in dotted-decimal notation
     */
    [[nodiscard]] std::string GetIp() const {
        return _ip;
    }

    /**
     * @brief Gets the underlying sockaddr_in structure.
     * @return Reference to the socket address structure
     */
    [[nodiscard]] const sockaddr_in& GetAddr() const {
        return _addr;
    }

    /**
     * @brief Sets the port number.
     * @param port Port number in host byte order
     */
    void SetPort(const in_port_t& port) {
        _port = port;
        _addr.sin_port = htons(port);
    }

    /**
     * @brief Sets the IP address.
     * @param ip IP address as string (e.g., "192.168.1.1")
     */
    void SetIp(std::string ip) {
        _addr.sin_addr.s_addr = inet_addr(ip.c_str());
        // move must be at last
        _ip = std::move(ip);
    }

    /**
     * @brief Sets the entire sockaddr_in structure and updates cached values.
     * @param addr Socket address structure to copy
     */
    void SetAddr(const sockaddr_in& addr) {
        _addr = addr;
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &_addr.sin_addr, buffer, sizeof buffer);
        _ip = buffer;
        _port = ntohs(_addr.sin_port);
    }

    /**
     * @brief Gets the size of the sockaddr_in structure.
     * @return Size in bytes (suitable for passing to socket API functions)
     */
    [[nodiscard]] socklen_t GetAddrLen() const {
        return sizeof _addr;
    }

    /**
     * @brief Compares two addresses for equality.
     * @param other Address to compare with
     * @return true if both IP and port match, false otherwise
     */
    bool operator==(const InetAddr& other) const {
        return _port == other._port && _ip == other._ip;
    }

    ~InetAddr() = default;
private:
    in_port_t _port;      ///< Port number in host byte order
    std::string _ip;      ///< IP address as string
    sockaddr_in _addr;    ///< Underlying socket address structure
};
