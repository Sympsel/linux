#pragma once

#include <pthread.h>
#include "Mutex.hpp"

/**
 * @brief Condition variable wrapper for thread synchronization.
 *
 * Provides a RAII interface for POSIX condition variables,
 * allowing threads to wait for notifications and coordinate execution.
 */
class Cond {
public:
    /**
     * @brief Constructs and initializes a condition variable.
     */
    Cond() : _cond() {
        pthread_cond_init(&_cond, nullptr);
    }

    /**
     * @brief Blocks the calling thread until the condition variable is notified.
     * @param mutex Mutex that must be locked before calling this method.
     *              The mutex is atomically released while waiting and re-acquired before returning.
     */
    void Wait(Mutex& mutex) {
        pthread_cond_wait(&_cond, mutex.origin());
    }

    /**
     * @brief Wakes up one thread waiting on this condition variable.
     * @note If multiple threads are waiting, only one will be woken (unspecified which one).
     */
    void NotifyOne() {
        pthread_cond_signal(&_cond);
    }

    /**
     * @brief Wakes up all threads waiting on this condition variable.
     */
    void NotifyAll() {
        pthread_cond_broadcast(&_cond);
    }

    /**
     * @brief Destructor that destroys the condition variable.
     */
    ~Cond() {
        pthread_cond_destroy(&_cond);
    }

private:
    pthread_cond_t _cond;
};
