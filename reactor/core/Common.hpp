#pragma once

#include <sys/epoll.h>

enum ERR {
    OF_CREATE = 1,
    OF_BIND = 2,
    OF_LISTEN = 3,
    OF_EPOLL = 4
};

enum EPOLL {
    ERROR = EPOLLERR,
    IN = EPOLLIN,
    OUT = EPOLLOUT,
    HUP = EPOLLHUP,
    ET = EPOLLET
};