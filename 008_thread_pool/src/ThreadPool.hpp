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
                // LockGuard lockguard(&_lock);
                while (IsTaskQueueEmpty()) {
                    ++_sleeper_cnt;
                    LOG(log_level_t::INFO) << "No task, Thread" << name << " sleep";
                    _cond.Wait(_lock);
                    LOG(log_level_t::INFO) << "Get a task, Thread" << name << " notified";
                    --_sleeper_cnt;
                }
                sleep(1);
                // T need a operator() to call
                task = PopHelper();
            }
            task();
        }
    }

   public:
    ThreadPool(int num = gnum) : _num(num), _isRunning(false), _sleeper_cnt() {
        for (int i = 0; i < _num; ++i) {
            _threads.emplace_back([this]() {
                this->ThreadRoutine();
            });
        }
    }

    ~ThreadPool() {
    }

    void Push(const T& task) {
        // LockGuard lockguard(&_lock);
        LockGuard lockguard(_lock);
        _q.emplace(task);
        if (_sleeper_cnt > 0)
            _cond.NotifyAll();
    }

    void Start() {
        if (_isRunning) return;
        for (auto& thread : _threads) {
            thread.start();
        }
        _isRunning = true;
    }

    void Stop() {
        if (_isRunning) {
            for (auto& thread : _threads) {
                thread.stop();
            }
            _isRunning = false;
        }
    }

    void Wait() {
        for (auto& thread : _threads) {
            thread.join();
        }
    }

   private:
    std::vector<Thread> _threads;
    int _num;
    bool _isRunning;

    std::queue<T> _q;
    Mutex _lock;
    Cond _cond;

    int _sleeper_cnt;
};