#include <iostream>
#include <mutex>
#include <thread>




int main() {
    std::mutex mtx;

    std::thread t1([&mtx]() {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate some work
        mtx.lock();
        std::cout << "Thread 1 has locked the mutex." << std::endl;
        mtx.unlock();
    });
    
    t1.join();
    return 0;
}
