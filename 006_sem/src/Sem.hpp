#pragma once

#include <pthread.h>

#include <iostream>
#include <semaphore.h>

class Semaphore {
   public:
    Semaphore(int val) : _val(val) {
        sem_init(&_sem, 0, _val);
    }

    void P() {
        sem_wait(&_sem);
    }

    void V() {
        sem_post(&_sem);
    }

    ~Semaphore() {
        sem_destroy(&_sem);
    }
   private:
    sem_t _sem;
    int _val;
};