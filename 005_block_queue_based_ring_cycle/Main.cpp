#include <iostream>
#include <thread>
#include "RingQueue.hpp"
#include <unistd.h>

int main() {
    std::unique_ptr<RingQueue<int>> ringqueue = std::make_unique<RingQueue<int>>();

    std::thread c([&ringqueue]() {
        while (true) {
            // get data

            int data;
            ringqueue->Pop(&data);
            std::cout << "consume a data: " << data << std::endl;
            sleep(1);
        }
    });
    std::thread p([&ringqueue]() {
        int data = 1;
        while (true) {
            ringqueue->Enqueue(data);
            std::cout << "product a data: " << data << std::endl;
        }
    });

    c.join();
    p.join();
    return 0;
}
