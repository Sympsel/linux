#pragma once
#include <pthread.h>

/**
 * @brief Mutex wrapper class for thread synchronization.
 *
 * Provides a RAII-compatible interface for POSIX mutex operations,
 * enabling mutual exclusion for shared resource access.
 */
class Mutex {
    public:
        /**
         * @brief Constructs and initializes a mutex.
         */
        Mutex() : _lock() {
            pthread_mutex_init(&_lock, nullptr);
        }

        /**
         * @brief Locks the mutex. If already locked, blocks until available.
         */
        void Lock() {
            pthread_mutex_lock(&_lock);
        }

        /**
         * @brief Returns the underlying POSIX mutex pointer.
         * @return Pointer to the pthread_mutex_t
         * @note Used for interoperability with POSIX threading functions
         */
        pthread_mutex_t* origin() {
            return &_lock;
        }

        /**
         * @brief Unlocks the mutex.
         */
        void Unlock() {
            pthread_mutex_unlock(&_lock);
        }

        /**
         * @brief Destructor that destroys the mutex.
         */
        ~Mutex() {
            pthread_mutex_destroy(&_lock);
        }

    private:
        pthread_mutex_t _lock;
};

/**
 * @brief RAII guard for automatic mutex locking and unlocking.
 *
 * Automatically locks a mutex upon construction and unlocks it upon destruction,
 * ensuring exception-safe lock management. Copy and move operations are disabled.
 */
class LockGuard {
   private:
    Mutex& _lock;

   public:
    /**
     * @brief Constructs a LockGuard and locks the associated mutex.
     * @param lock Reference to the mutex to lock
     */
    explicit LockGuard(Mutex& lock) : _lock(lock) {
        _lock.Lock();
    }

    /**
     * @brief Destructor that unlocks the associated mutex.
     */
    ~LockGuard() {
        _lock.Unlock();
    }

    // RAII was prohibited to copy or assign
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    LockGuard(LockGuard&&) = delete;
    LockGuard& operator=(LockGuard&&) = delete;
};
