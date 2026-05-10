#pragma once

#include <csignal>
#include <cstring>
#include <fcntl.h>

#include "unistd.h"

#include "../../utils/module/SymLog.h"

// 将进程变为守护进程

inline void Deamon() {
    // 防止客户端断开连接时，服务端会收到SIGPIPE信号
    signal(SIGPIPE, SIG_IGN);
    // 忽略子进程退出信号, 让子进程变成孤儿进程
    signal(SIGCHLD, SIG_IGN);
    if (const pid_t pid = fork(); pid < 0) {
        LOG_ERROR() << "fork error: " << strerror(errno);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    setsid();

    if (const int fd = open("/dev/null", O_RDWR); fd >= 0) {
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
    } else {
        LOG_ERROR() << "open /dev/null error: " << strerror(errno);
    }
}
