#pragma once

#include <queue>
#include <vector>

#include "Mutex.hpp"
#include "Cond.hpp"
#include "Thread.hpp"
#include "../utils/Log.hpp"

using namespace sym;

static int gnum = 5;
static bool enable_log = false;

template <class T>
class ThreadPool {
   private:
    enum {
        RUNNING,
        STOP,
        QUIT
    };
    bool IsTaskQueueEmpty() {
        return _q.empty();
    }

    T PopHelper() {
        T t = _q.front();
        _q.pop();
        return t;
    }

    void ThreadRoutine() {
        char name[64];
        pthread_getname_np(pthread_self(), name, sizeof name);
        while (true) {
            T task;
            {
                LockGuard lockguard(_lock);
                while (IsTaskQueueEmpty() && _status == RUNNING) {
                    ++_sleeper_cnt;
                    if (enable_log) {
                        LOG_DEBUG() << "No task, Thread" << name << " sleep";
                    }
                    _cond.Wait(_lock);
                    if (enable_log) {
                        LOG_DEBUG() << "Get a task, Thread" << name << " notified";
                    }
                    --_sleeper_cnt;
                }
                if (IsTaskQueueEmpty() && _status != RUNNING) {
                    if (enable_log) {
                        LOG(log_level_t::INFO) << "Thread " << name << " exit.";
                    }
                    break;
                }
                task = PopHelper();
            }
            task();
        }
    }

    explicit ThreadPool(const int num) : _num(num), _status(STOP), _sleeper_cnt() {
        for (int i = 0; i < _num; ++i) {
            _threads.emplace_back([this]() {
                this->ThreadRoutine();
            });
        }
    }

   public:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    static ThreadPool<T>& GetInstance(int num = gnum) {
        static ThreadPool<T> instance(num);
        instance.Start();
        return instance;
    }

    ~ThreadPool() = default;

    void Push(const T& task) {
        LockGuard lockguard(_lock);
        _q.emplace(task);
        if (_sleeper_cnt > 0)
            _cond.NotifyAll();
    }

    void Start() {
        if (_status == RUNNING) return;
        for (auto& thread : _threads) {
            thread.start();
        }
        _status = RUNNING;
    }

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

    void Wait() {
        for (auto& thread : _threads) {
            thread.join();
        }
    }

    void Enqueue(const T& task) {
        LockGuard lockguard(_lock);
        if (_status == QUIT) {
            LOG_WARN() << "Threadpool is going to exit.";
            return;
        }
        _q.emplace(task);
        if (_sleeper_cnt > 0) {
            _cond.NotifyOne();
        }
    }

    void Quit() {
        if (_status == QUIT) {
            LOG_INFO() << "Threadpool is going to exit.";
            LockGuard lockguard(_lock);
            _status = QUIT;
            _cond.NotifyAll();
        }
    }

   private:
    std::vector<Thread> _threads;
    int _num;
    int _status;

    std::queue<T> _q;
    Mutex _lock;
    Cond _cond;

    int _sleeper_cnt;
};