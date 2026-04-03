#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

# if defined (__linux__)
#include <unistd.h>
# elif defined (_WIN32)
#include <windows.h>
# endif

void test1() {
    std::chrono::system_clock::time_point t1 = std::chrono::system_clock::now();
    std::cout << "Start: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::chrono::system_clock::time_point t2 = std::chrono::system_clock::now();
    std::cout << "Done: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms" << std::endl;
}

bool flag = true;
void print(int n, int i) {
    std::cout << "Start: " << std::this_thread::get_id() << std::endl;
    for (; i < n; ++i) {
        std::cout << i << " \n"[i == n - 1];
        if (i == 5 && flag) {
            std::this_thread::yield();
            flag = false;
        }
    }
    std::cout << "Done" << std::endl;
}

void test2() {
    std::thread t1{print, 10, 0};
    std::thread t2{print, 20, 10};
    t1.join();
    t2.join();
}

int main() {
    test1();
    return 0;
}