#include <iostream>
#include <thread>
#include <atomic>

int ticket = 1000000;  // 100万
int sold_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void sell_fast() {
    int my_count = 0;
    while (true) {
        pthread_mutex_lock(&mutex);
        int t = ticket;      // 读
        if (t <= 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        ticket = t - 1;      // 写（竞争点！）
        pthread_mutex_unlock(&mutex);
        ++my_count;
        
    }
    __sync_fetch_and_add(&sold_count, my_count);
}

int main() {
    std::thread t1(sell_fast);
    std::thread t2(sell_fast);
    t1.join(); t2.join();
    
    std::cout << "初始: 1000000, 售出: " << sold_count 
              << ", 剩余: " << ticket << std::endl;
    // 典型输出：售出 > 1000000（超卖！）或 < 1000000（丢失）
    return 0;
}


//  #include <iostream>
//  #include <thread>
//  #include <unistd.h>

//  int ticket = 1000;

//  int main() {

//         auto func = [](std::string name) {
//             while (ticket > 0) {
//                 std::cout << name << ": " << ticket-- << std::endl;
//                 // usleep(1000); // Sleep for 1 millisecond
//             }
//         };
//         std::thread t1(func, "Thread-1");
    
//         std::thread t2(func, "Thread-2");
    
//         t1.join();
//         t2.join();
    
//         return 0;
//  }