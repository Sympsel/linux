#pragma once

#include <mutex>
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

    void Pop(T* out) {
        // check semaphore before lock, otherwise may cause deadlock
        sem_wait(&_data_sem);
        {
            std::lock_guard<std::mutex> lock(_cons_mutex);
            *out = _buffer[_cons_step];
            _cons_step = (_cons_step + 1) % _cap;
        }
        sem_post(&_space_sem);
    }

    void Enqueue(const T& in) {
        sem_wait(&_space_sem); // P
        {
            std::lock_guard<std::mutex> lock(_prod_mutex);
            _buffer[_prod_step] = in;
            _prod_step = (_prod_step + 1) % _cap;
        }
        sem_post(&_data_sem);  // V
    }

   private:
    std::vector<T> _buffer;
    int _cap;
    int _prod_step;
    int _cons_step;

    sem_t _space_sem;
    sem_t _data_sem;

    std::mutex _prod_mutex;
    std::mutex _cons_mutex;
};
