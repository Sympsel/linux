#pragma once

#include <sys/syscall.h>
#include <unistd.h>

#include <functional>
#include <iostream>

using func_t = std::function<void()>;

namespace {
int gcnt = 0;
}

class Thread {
   private:
    enum ThreadState {
        NEW,
        RUNNING,
        STOPPED
    };

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
        t->_status = STOPPED;
        return nullptr;
    }

   public:
    Thread(func_t f) : _func(f), _joinable(true), _status(NEW) {
        _name = "Thread-" + std::to_string(++gcnt);
    }

    void start() {
        if (_status == NEW) {
            pthread_create(&_tid, nullptr, routine, this);
            _status = RUNNING;
            std::cout << "Thread " << _name << " is started." << std::endl;
        }
    }

    void stop() {
        if (_status == RUNNING) {
            // we should ensure it was running, because we can't cancel it twice
            pthread_cancel(_tid);
            _status = STOPPED;
        }
        std::cout << "Thread " << _name << " is stopped." << std::endl;
    }

    void detach() {
        if (_joinable && _status == RUNNING) {
            _joinable = false;
            pthread_detach(_tid);
        }
    }

    void join() {
        if (_joinable && _status == RUNNING) {
            pthread_join(_tid, nullptr);
            _joinable = false;
        }
    }

    ~Thread() = default;

   private:
    func_t _func;
    pthread_t _tid;
    pid_t _pid;
    pid_t _lwpid;  // Lightweight Process ID
    std::string _name;
    bool _joinable;
    ThreadState _status;
};