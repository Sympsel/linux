#include <iostream>
#include <print>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

void setNonBlock(int fd) {
    // 获取当前文件描述符的标志位
    const int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    fd_set rfds;
    timeval timeout;
    int fd_stdin = STDIN_FILENO;
    std::println("等待用户输入，5秒后超时");

    while (true) {
        FD_ZERO(&rfds);
        FD_SET(fd_stdin, &rfds);

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        int ret = select(fd_stdin + 1, &rfds, nullptr, nullptr, &timeout);
        if (ret == -1) {
            std::println("出错");
            break;
        }
        if (ret == 0) {
            std::println("超时，无数据可读");
            continue;
        }
        if (FD_ISSET(fd_stdin, &rfds)) {
            char buff[1024];
            size_t n = read(fd_stdin, buff, sizeof buff - 1);
            if (n > 0) {
                buff[n] = '\0';
                std::println("读到：{}", buff);
            }
        }
    }
    return 0;
}
