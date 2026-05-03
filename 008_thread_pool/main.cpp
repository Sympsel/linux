#include <unistd.h>

#include <vector>

#include "Task.hpp"
#include "ThreadPool.hpp"

int main() {
    USE_CONSOLE_LOG_STRATEGY();
    // std::unique_ptr<ThreadPool<task_t>> tp = std::make_unique<ThreadPool<task_t>>(5);
    ThreadPool<task_t>* tp = ThreadPool<task_t>::GetInstance();
    tp->Start();
    std::vector<int> tasks{
        1, 2, 1, 1, 2, 1, 2, 2};

    for (auto taskid : tasks) {
        if (taskid == 1) {
            tp->Push(HttpParse);
        } else if (taskid == 2) {
            tp->Push(JsonParse);
        }
        sleep(1);
    }
    tp->Stop();
    tp->Wait();
    return 0;
}