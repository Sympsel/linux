#include "../../include/network/SocketUtils.hpp"
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

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
