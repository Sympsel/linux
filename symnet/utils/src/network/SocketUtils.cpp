#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "../../include/network/SocketUtils.h"
#include "../../include/slog/Log.h"

int SocketUtils::CreateSocket(const int domain, const int type, const int protocol) {
    const int sockfd = socket(domain, type, protocol);
    if (sockfd < 0) {
        return -1;
    }
    return sockfd;
}

bool SocketUtils::Bind(const int sockfd, const InetAddr &peer) {
    if (const int ret = bind(sockfd, reinterpret_cast<const sockaddr *>(&peer.GetAddr()),
                             peer.GetAddrLen()); ret < 0) {
        return false;
    }
    return true;
}

bool SocketUtils::Listen(const int sockfd, const int backlog) {
    return listen(sockfd, backlog) >= 0;
}

int SocketUtils::Accept(const int listen_sockfd, InetAddr &addr_client) {
    sockaddr_in temp{};
    socklen_t len = sizeof temp;
    const int connect_sockfd = accept(listen_sockfd, reinterpret_cast<sockaddr *>(&temp), &len);
    if (connect_sockfd < 0) {
        return -1;
    }
    addr_client.SetAddr(temp);
    return connect_sockfd;
}

bool SocketUtils::Connect(const int sockfd, const InetAddr &peer) {
    return connect(sockfd, reinterpret_cast<const sockaddr *>(&peer.GetAddr()), peer.GetAddrLen()) >= 0;
}


std::string SocketUtils::Read(const int sockfd, const size_t max_size) {
    char inbuffer[max_size];
    const ssize_t n = read(sockfd, inbuffer, sizeof inbuffer - 1);
    if (n <= 0) {
        return {};
    }
    inbuffer[n] = '\0';
    return inbuffer;
}

bool SocketUtils::Read(const int sockfd, std::string& str_to_fill, const size_t max_size) {
    char inbuffer[max_size];
    const ssize_t n = read(sockfd, inbuffer, sizeof inbuffer - 1);
    if (n <= 0) {
        return false;
    }
    inbuffer[n] = '\0';
    str_to_fill = inbuffer;
    return true;
}

ssize_t SocketUtils::Write(const int sockfd, const std::string &message) {
    return write(sockfd, message.c_str(), message.size());
}

bool SocketUtils::Close(const int sockfd) {
    if (sockfd >= 0) {
        if (const int ret = close(sockfd); ret < 0) {
            return false;
        }
        return true;
    }
    return true;
}

ssize_t SocketUtils::SendTo(const int sockfd, const std::string &data, const InetAddr &peer) {
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

bool SocketUtils::RecvFrom(const int sockfd, std::string& str_to_fill, InetAddr &sender_to_fill) {
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

bool SocketUtils::RecvFrom(const int sockfd, std::string& str_to_fill) {
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

bool SocketUtils::SetReuseAddr(const int sockfd) {
    constexpr int opt = 1;
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) >= 0;
}

bool SocketUtils::SetReusePort(const int sockfd) {
    constexpr int opt = 1;
#ifdef SO_REUSEPORT
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) >= 0;
#else
    return false;
#endif
}

bool SocketUtils::SetNonBlock(const int sockfd) {
    const int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) return false;
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) >= 0;
}
