#include "BlockQueue.hpp"
#include <pthread.h>
#include <iostream>
#include <unistd.h>

void* Consumer(void* args) {
    BlockQueue<int>* pq = static_cast<BlockQueue<int>*>(args);
    while (true) {
        int data = 0;
        pq->Pop(&data);
        std::cout << "Consumed: " << data << std::endl;
        sleep(1);
    }
}

void* Producer(void* args) {
    BlockQueue<int>* pq = static_cast<BlockQueue<int>*>(args);
    int data = 10;
    while (true) {
        pq->Push(data);
        std::cout << "Produced: " << data << std::endl;
        data++;
        sleep(1);
    }
}

int main() {
    BlockQueue<int>* pq = new BlockQueue<int>();
    pthread_t prod_tid, cons_tid;
    pthread_create(&prod_tid, nullptr, Producer, pq);
    pthread_create(&cons_tid, nullptr, Consumer, pq);
    pthread_join(prod_tid, nullptr);
    pthread_join(cons_tid, nullptr);
    delete pq;
}
