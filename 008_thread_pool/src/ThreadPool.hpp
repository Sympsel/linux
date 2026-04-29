#pragma once

#include <queue>
#include <vector>

#include "Cond.hpp"
#include "Mutex.hpp"
#include "Thread.hpp"
#include "log.hpp"
using namespace sym;

namespace {
const int gnum = 5;
}

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
                    LOG(log_level_t::INFO) << "No task, Thread" << name << " sleep";
                    _cond.Wait(_lock);
                    LOG(log_level_t::INFO) << "Get a task, Thread" << name << " notified";
                    --_sleeper_cnt;
                }
                if (IsTaskQueueEmpty() && _status != RUNNING) {
                    LOG(log_level_t::INFO) << "Thread " << name << " exit.";
                    break;
                }
                // T need a operator() to call
                task = PopHelper();
            }
            task();
        }
    }

   public:
    ThreadPool(int num = gnum) : _num(num), _status(STOP), _sleeper_cnt() {
        for (int i = 0; i < _num; ++i) {
            _threads.emplace_back([this]() {
                this->ThreadRoutine();
            });
        }
    }

    ~ThreadPool() {
    }

    void Push(const T& task) {
        LockGuard lockguard(_lock);
        _q.emplace(task);
        if (_sleeper_cnt > 0)
            _cond.NotifyAll();
    }

    void Start() {
        LockGuard lockguard(_lock);
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
            LOG(log_level_t::WARNING) << "Threadpool is going to exit.";
            return;
        }
        _q.emplace(task);
        if (_sleeper_cnt > 0) {
            _cond.NotifyOne();
        }
    }

    void Quit() {
        if (_status == QUIT) {
            LOG(log_level_t::INFO) << "Threadpool is going to exit.";
            LockGuard lockguard(_lock);
            _status = QUIT;
            _cond.NotifyAll();
        }
    }

   private:
    std::vector<Thread> _threads;
    int _num;
    // bool _isRunning;
    int _status;

    std::queue<T> _q;
    Mutex _lock;
    Cond _cond;

    int _sleeper_cnt;
};