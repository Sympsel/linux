#pragma once

#include <vector>
#include <semaphore.h>

static const int gdefaultcap = 5;

template<class T>
class RingQueue {
   public:
    RingQueue(int cap = gdefaultcap) : _buffer(cap), _cap(cap), _prod_step(0), _cons_step(0) {
        sem_init(&_space_sem, 0, cap);
        sem_init(&_data_sem, 0, 0);
    }

    ~RingQueue() {
        sem_destroy(&_space_sem);
        sem_destroy(&_data_sem);
    }

    void Pop(int* out) {}

    void Enqueue(const T& in) {
        sem_wait(&_space_sem); // P
        _buffer[_prod_step] = in;
        _prod_step = (_prod_step + 1) % _cap;
        sem_post(&_data_sem);  // V
    }

   private:
    std::vector<T> _buffer;
    int _cap;
    int _prod_step;
    int _cons_step;

    sem_t _space_sem;
    sem_t _data_sem;
};
