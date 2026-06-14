#include <memory>
#include "epoll/EpollServer.hpp"

constexpr uint16_t gPort = 8080;

int main() {
    const auto epollServer = std::make_unique<EpollServer>(gPort);
    epollServer->start();
    return 0;
}