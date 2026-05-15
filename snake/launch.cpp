#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdio>

int main() {
    // 1. 编译游戏
    const std::string compile = "mkdir -p out &&"
            " g++ -std=c++20 -O2 "
            "-o out/snake "
            "src/main.cpp "
            "-lncurses -ljsoncpp";
    if (system(compile.c_str()) != 0) {
        printf("Compile failed!\n");
        return 1;
    }

    // 2. 启动固定大小的 xterm
    const pid_t pid = fork();
    if (pid == 0) {
        execlp("xterm", "xterm",
               "-title", "Snake Game",
               "-geometry", "62x24",
               "-fa", "monospace:size=14:bold",
               "+sb",
               "-bg", "black",
               "-fg", "green",
               "-e", "out/snake",
               nullptr);
        perror("xterm failed");
        _exit(1);
    }

    // 3. 父进程等待
    int status;
    waitpid(pid, &status, 0);
    printf("Game exited.\n");
    return 0;
}

// mkdir -p out && g++ -std=c++20 -O2 -o out/snake src/main.cpp -lncurses -ljsoncpp
