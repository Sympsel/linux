#pragma once

#include <iostream>
#include <functional>
#include <unistd.h>
#include <sys/syscall.h>

using func_t = std::function<void()>;

enum ThreadState {
    THREAD_NEW,
    THREAD_RUNNING,
    THREAD_STOPPED
};

namespace {
    int gcnt;
}


class Thread {
private:
    void get_proc_id() {
        _pid = getpid();
    }

    void setlwp() {
        _lwpid = syscall(SYS_gettid);
    }

    static void* routine(void* args) {
        Thread* t = static_cast<Thread*>(args);
        t->get_proc_id();
        t->setlwp();
        t->_func();
        t->_status = THREAD_STOPPED;
        return nullptr;
    }
   public:
    Thread(func_t f) : _func(f), _joinable(true), _status(THREAD_NEW) {
        _name = "Thread-" + std::to_string(++gcnt);
    }

    void start() {
        if (_status == THREAD_NEW) {
            pthread_create(&_tid, nullptr, routine, this);
            _status = THREAD_RUNNING;
            std::cout << "Thread " << _name << " is started." << std::endl;
        }
    }

    void stop() {
        if (_status == ThreadState::THREAD_RUNNING) {
            // we should ensure it was running, because we cant't cancel it twice
            pthread_cancel(_tid);
            _status = THREAD_STOPPED;
        }
        std::cout << "Thread " << _name << " is stopped." << std::endl;
    }

    void detach() {
        if (_joinable && _status == THREAD_RUNNING) {
            _joinable = false;
            pthread_detach(_tid);
        }
    }

    void join() {
        if (_joinable && _status == THREAD_RUNNING) {
            pthread_join(_tid, nullptr);
            _joinable = false;
        }
    }

    ~Thread() {}

   private:
    func_t _func;
    pthread_t _tid;
    pid_t _pid;
    pid_t _lwpid; // Light Weight Process ID
    std::string _name;
    bool _joinable;
    ThreadState _status;

};