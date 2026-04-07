#include <iostream>
#include "Thread.hpp"

void hello() {
    while (true) {
        std::cout << "Hello, World!" << std::endl;
        sleep(1);
    }
}

int main() {
    std::vector<Thread*> threads;
    for (int i{}; i < 5; ++i) {
        Thread* t = new Thread(hello);
        t->start();
        threads.emplace_back(t);
    }
    for (auto t : threads) {
        t->join();
        sleep(1);
        t->stop();
        delete t;
    }

    std::cout << "Main thread exit." << std::endl;
    return 0;
}
