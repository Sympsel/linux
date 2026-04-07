#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

/*
    Thread-local storage variable
    Simplified example of using thread-local storage in C++.
    Each thread will have its own instance of `thread_local_var`,
    and changes to it in one thread will not affect the value in other threads.
*/
__thread int thread_local_var = 0;
__thread int lwp_id = 0;

pid_t getlwp() {
    return syscall(SYS_gettid);
}

void* func1(void* arg) {
    lwp_id = getlwp();
    std::cout << "Hello from thread 1" << std::endl;
    return nullptr;
}

int main() {
    pthread_t thread1;
    pthread_create(&thread1, nullptr, func1, nullptr);
    pthread_join(thread1, nullptr);
    std::cout << "Hello, World" << std::endl;
    return 0;
}
