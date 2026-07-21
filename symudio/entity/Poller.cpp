#include "Poller.h"

void Poller::poll(std::vector<Channel *> &actives) {
    int nfds = epoll_wait(_epFd, _epEvents, sizeof _epEvents, -1);
    if (nfds < 0) {
        if (errno == EINTR) {
            return;
        }
        LOG_FATAL() << std::format("Epoll Wait Error, Code: {}", strerror(errno));
        exit(EXIT_FAILURE);
    }
    for (int i{}; i < nfds; ++i) {
        if (!_channels.contains(_epEvents[i].data.fd)) {
            LOG_FATAL() << std::format("Epoll Fd Not Found!");
            exit(EXIT_FAILURE);
        }
        _channels[i]->setREvent(_epEvents[i].events);
        actives.emplace_back(_channels[i]);
    }
}
