#include <pthread.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "my_task.hpp"

static int thread_num;

void Usage(const std::string& argv) {
    std::string argv0 = argv;
    ssize_t pos = argv0.find_last_of("/");
    argv0 = argv0.substr(pos + 1);
    std::cout << "========================================" << std::endl;
    std::cout << "Usage: \n\n\t";
    std::cout << argv0 << " <thread_num>" << std::endl;
    std::cout << "========================================" << std::endl;
}

void* run1(void* args) {
    const MyTask* task = static_cast<const MyTask*>(args);
    // (*task)();
    task->operator()();
    while (true) {
        sleep(1);
    }
    return nullptr;
}

void test1() {
    std::vector<pthread_t> tids;
    for (int i{1}; i <= thread_num; ++i) {
        pthread_t tid;
        std::string tname = "thread_" + std::to_string(i);
        MyTask* task = new MyTask(tname, i, i * i);
        pthread_create(&tid, nullptr, run1,
                       (static_cast<void*>(task)));
        tids.push_back(tid);
        sleep(1);
    }
    for (auto& tid : tids) {
        std::cout << "created thread " << tid << std::endl;
    }
    while (true) {
        sleep(1);
    }
}

void* run2(void* args) {
    std::string* cmd = static_cast<std::string*>(args);
    pthread_t tid;
    system(cmd->c_str());
    delete cmd;
    int cnt{5};
    while (cnt--) {
        sleep(1);
    }
    pthread_exit(nullptr);
}

void test2() {
    char dir[1024];
    getcwd(dir, 1024);
    std::string* cmd = new std::string("ls -a ");
    *cmd += dir;

    pthread_t tid;
    pthread_create(&tid, nullptr, run2,
                   (static_cast<void*>(cmd)));

    int cnt{6};
    while (cnt--) {
        sleep(1);
    }
}

void* xxx(void** ret) {
    *ret = (void*)10;
    return (void*)20;
}

void test3() {
    void* a;
    printf("%p\n", a);
 
    long long b = (long long)a;
    printf("%lld\n", b);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::string argv0 = argv[0];
        Usage(argv0);
        return 1;
    }

    // test1();
    // test2();
    test3();
    return 0;
}
