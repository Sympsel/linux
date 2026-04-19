#include <iostream>
#include <thread>
#include "src/RingQueue.hpp"
#include <unistd.h>
#include <functional>
#include <vector>

using task_t = std::function<void()>;

void Task() {
    std::cout << "a task handle by thread: " << std::this_thread::get_id() << std::endl;

}

int main() {
    std::unique_ptr<RingQueue<task_t>> ringqueue = std::make_unique<RingQueue<task_t>>();

    auto consumer_cb = [&ringqueue]() {
        while (true) {
            // get task

            task_t t;
            ringqueue->Pop(&t);
            t();
            sleep(1);
        }
    };

    auto producer_cb = [&ringqueue]() {
        task_t data = Task;
        while (true) {
            ringqueue->Enqueue(data);
            std::cout << "product a task" << std::endl;
            sleep(1);
        }
    };

    // for (int i = 0; i < 3; ++i) {
    //     std::thread(consumer_cb).join();
    // }
    // for (int i = 0; i < 2; ++i) {
    //     std::thread(producer_cb).join();
    // }

    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(consumer_cb);
    }
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(producer_cb);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }


    return 0;
}
