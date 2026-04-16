#ifndef __COND_CPP
#define __COND_CPP

#include <pthread.h>
#include "Mutex.hpp"

class Cond {
public:
    Cond() {
        pthread_cond_init(&_cond, nullptr);
    }

    void Wait(Mutex& mutex) {
        pthread_cond_wait(&_cond, mutex.orgin());
    }

    void NotifyOne() {
        pthread_cond_signal(&_cond);
    }

    void NotifyAll() {
        pthread_cond_broadcast(&_cond);
    }

    ~Cond() {
        pthread_cond_destroy(&_cond);
    }

private:
    pthread_cond_t _cond;
};

#endif
