#pragma once

#include <queue>
#include <vector>

#include "Mutex.hpp"
#include "Cond.hpp"
#include "Thread.hpp"

using namespace sym;

/**
 * @brief Template class implementing a thread pool pattern.
 *
 * Manages a fixed number of worker threads that execute tasks from a shared queue.
 * Implements the singleton pattern for easy access and resource management.
 *
 * @tparam T Task type (should be callable, e.g., std::function<void()>)
 */
template <class T>
class ThreadPool {
   private:
    enum {
        RUNNING,    ///< Pool is actively processing tasks
        STOP,       ///< Pool is stopped but not destroyed
        QUIT        ///< Pool is shutting down
    };

    /**
     * @brief Checks if the task queue is empty.
     * @return true if no tasks are queued, false otherwise
     */
    bool IsTaskQueueEmpty() {
        return _q.empty();
    }

    /**
     * @brief Removes and returns the front task from the queue.
     * @return The next task to execute
     */
    T PopHelper() {
        T t = _q.front();
        _q.pop();
        return t;
    }

    /**
     * @brief Main loop executed by each worker thread.
     *
     * Continuously waits for tasks, executes them, and handles shutdown signals.
     */
    void ThreadRoutine() {
        char name[64];
        pthread_getname_np(pthread_self(), name, sizeof name);
        while (true) {
            T task;
            {
                LockGuard lock_guard(_lock);
                while (IsTaskQueueEmpty() && _status == RUNNING) {
                    ++_sleeper_cnt;
                    if (Conf::threadpool_enable_log) {
                        LOG_DEBUG() << "No task, Thread" << name << " sleep";
                    }
                    _cond.Wait(_lock);
                    if (Conf::threadpool_enable_log) {
                        LOG_DEBUG() << "Get a task, Thread" << name << " notified";
                    }
                    --_sleeper_cnt;
                }
                if (IsTaskQueueEmpty() && _status != RUNNING) {
                    if (Conf::threadpool_enable_log) {
                        LOG(log_level_t::INFO) << "Thread " << name << " exit.";
                    }
                    break;
                }
                task = PopHelper();
            }
            task();
        }
    }

    /**
     * @brief Private constructor that initializes the thread pool.
     * @param num Number of worker threads to create
     */
    explicit ThreadPool(const int num) : _num(num), _status(STOP), _sleeper_cnt() {
        for (int i = 0; i < _num; ++i) {
            _threads.emplace_back([this]() {
                this->ThreadRoutine();
            });
        }
    }

   public:
    // Disable copying and moving
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /**
     * @brief Gets the singleton instance of the thread pool.
     * @param num Number of threads (used only on first call, default: gnum)
     * @return Reference to the singleton thread pool instance
     */
    static ThreadPool<T>& GetInstance(int num = Conf::threadpool_default_size) {
        static ThreadPool<T> instance(num);
        instance.Start();
        return instance;
    }

    ~ThreadPool() = default;

    /**
     * @brief Adds a task to the thread pool queue (thread-safe).
     * @param task Task to be executed by a worker thread
     */
    void Push(const T& task) {
        LockGuard lock_guard(_lock);
        _q.emplace(task);
        if (_sleeper_cnt > 0)
            _cond.NotifyAll();
    }

    /**
     * @brief Starts all worker threads in the pool.
     * @note Has no effect if the pool is already running.
     */
    void Start() {
        if (_status == RUNNING) return;
        for (auto& thread : _threads) {
            thread.start();
        }
        _status = RUNNING;
    }

    /**
     * @brief Stops all worker threads.
     *
     * Signals threads to stop and wakes up any sleeping threads.
     * Threads will exit after completing their current tasks.
     */
    void Stop() {
        if (_status == RUNNING) {
            for (auto& thread : _threads) {
                thread.stop();
            }
            _status = STOP;
            // to handle if there are threads sleeping and none is active
            if (_sleeper_cnt > 0) {
                _cond.NotifyAll();
            }
        }
    }

    /**
     * @brief Waits for all worker threads to complete.
     * @note Should be called after Stop() to ensure clean shutdown.
     */
    void Wait() {
        for (auto& thread : _threads) {
            thread.join();
        }
    }

    /**
     * @brief Adds a task to the queue with shutdown protection.
     * @param task Task to enqueue
     * @note Warns and rejects tasks if the pool is quitting.
     */
    void Enqueue(const T& task) {
        LockGuard lock_guard(_lock);
        if (_status == QUIT) {
            LOG_WARN() << "Threadpool is going to exit.";
            return;
        }
        _q.emplace(task);
        if (_sleeper_cnt > 0) {
            _cond.NotifyOne();
        }
    }

    /**
     * @brief Initiates graceful shutdown of the thread pool.
     *
     * Sets the status to QUIT and notifies all threads to exit.
     * New tasks will be rejected.
     */
    void Quit() {
        if (_status == QUIT) {
            LOG_INFO() << "Threadpool is going to exit.";
            LockGuard lock_guard(_lock);
            _status = QUIT;
            _cond.NotifyAll();
        }
    }

   private:
    std::vector<Thread> _threads;   ///< Worker threads
    int _num;                        ///< Number of threads
    int _status;                     ///< Current pool status

    std::queue<T> _q;               ///< Task queue
    Mutex _lock;                     ///< Mutex for thread-safe queue access
    Cond _cond;                      ///< Condition variable for task notification

    int _sleeper_cnt;               ///< Count of threads currently waiting for tasks
};
