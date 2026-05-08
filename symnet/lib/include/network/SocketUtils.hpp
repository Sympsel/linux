#pragma once

#include <cstring>

#include "../utils/InetAddr.hpp"

constexpr int DEFAULT_BACKLOG = 32;
static constexpr int default_buffer_length = 1024;

class SocketUtils {
public:
    SocketUtils() = delete;

    static int CreateSocket(const int domain, const int type, const int protocol = 0) {
        const int sockfd = socket(domain, type, protocol);
        if (sockfd < 0) {
            return -1;
        }
        return sockfd;
    }

    static bool Bind(const int sockfd, const InetAddr &peer) {
        if (const int ret = bind(sockfd, reinterpret_cast<const sockaddr *>(&peer.GetAddr()),
                                 peer.GetAddrLen()); ret < 0) {
            return false;
        }
        return true;
    }

    static bool Listen(const int sockfd, const int backlog) {
        return listen(sockfd, backlog) >= 0;
    }

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

    static bool Connect(const int sockfd, const InetAddr &peer) {
        return connect(sockfd, reinterpret_cast<const sockaddr *>(&peer.GetAddr()), peer.GetAddrLen()) >= 0;
    }

    static std::string Read(const int sockfd) {
        char inbuffer[DEFAULT_BACKLOG];
        const ssize_t n = read(sockfd, inbuffer, sizeof inbuffer - 1);
        if (n <= 0) {
            return {};
        }
        inbuffer[n] = '\0';
        return inbuffer;
    }

    static ssize_t Write(const int sockfd, const std::string &message) {
        return write(sockfd, message.c_str(), message.size());
    }

    static bool Close(const int sockfd) {
        if (sockfd >= 0) {
            if (const int ret = close(sockfd); ret < 0) {
                return false;
            }
            return true;
        }
        return true;
    }

    // UDP
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

    static bool RecvFrom(const int sockfd, std::string& str_to_fill, InetAddr &sender_to_fill) {
        char inbuffer[default_buffer_length];
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

    static bool RecvFrom(const int sockfd, std::string& str_to_fill) {
        char inbuffer[default_buffer_length];
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

    // === 配置选项 ===
    static bool SetReuseAddr(int sockfd);

    static bool SetReusePort(int sockfd);

    static bool SetNonBlock(int sockfd);
};
