#ifndef __MUTEX_HPP
#define __MUTEX_HPP

#include <pthread.h>

class Mutex {
    public:
        Mutex() {
            pthread_mutex_init(&_lock, nullptr);
        }

        void Lock() {
            pthread_mutex_lock(&_lock);
        }

        pthread_mutex_t* orgin() {
            return &_lock; 
        }

        void Unlock() {
            pthread_mutex_unlock(&_lock);
        }

        ~Mutex() {
            pthread_mutex_destroy(&_lock);
        }

    private:
        pthread_mutex_t _lock;
};

class LockGuard {
    private:
        Mutex* _lock;
    public:
        LockGuard(Mutex* lock) : _lock(lock) {
            _lock->Lock();
        }

        ~LockGuard() {
            _lock->Unlock();
        }
};

#endif // !__MUTEX_HPP
