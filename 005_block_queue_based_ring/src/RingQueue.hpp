#pragma once

#include <mutex>
#include <vector>
// #include <semaphore.h>
#include <semaphore>

static const int gdefaultcap = 5;

template<class T>
class RingQueue {
   public:
    RingQueue(int cap = gdefaultcap)
    : _buffer(cap), _cap(cap), _prod_step(0), _cons_step(0)
    , _space_sem(cap), _data_sem(0) {}

    ~RingQueue() = default;

    void Pop(T* out) {
        // check semaphore before lock, otherwise may cause deadlock
        _data_sem.acquire();
        // sem_wait(&_data_sem);
        {
            std::lock_guard<std::mutex> lock(_cons_mutex);
            *out = _buffer[_cons_step];
            _cons_step = (_cons_step + 1) % _cap;
        }
        // sem_post(&_space_sem);
        _space_sem.release();
    }

    void Enqueue(const T& in) {
        _space_sem.acquire(); // P
        {
            std::lock_guard<std::mutex> lock(_prod_mutex);
            _buffer[_prod_step] = in;
            _prod_step = (_prod_step + 1) % _cap;
        }
        _data_sem.release();  // V
    }

   private:
    std::vector<T> _buffer;
    int _cap;
    int _prod_step;
    int _cons_step;
    
    std::counting_semaphore<gdefaultcap> _space_sem;
    std::counting_semaphore<gdefaultcap> _data_sem;
    std::mutex _prod_mutex;
    std::mutex _cons_mutex;
};
