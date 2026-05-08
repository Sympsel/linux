#pragma once

#include <sys/syscall.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <utility>

using func_t = std::function<void()>;

static int default_cnt = 0;

static constexpr bool enable_thread_log = false;

class Thread {
private:
    enum ThreadState {
        NEW,
        RUNNING,
        STOPPED
    };

    void GetPid() {
        _pid = getpid();
    }

    void SetLightWightPid() {
        _lwpid = static_cast<pid_t>(syscall(SYS_gettid));
    }

    static void *routine(void *args) {
        auto *t = static_cast<Thread *>(args);
        t->GetPid();
        t->SetLightWightPid();
        t->_func();
        t->_status = STOPPED;
        return nullptr;
    }

public:
    explicit Thread(func_t f) : _func(std::move(f)), _tid(0), _pid(0), _lwpid(0), _joinable(true), _status(NEW) {
        _name = "Thread-" + std::to_string(++default_cnt);
    }

    void start() {
        if (_status == NEW) {
            pthread_create(&_tid, nullptr, routine, this);
            _status = RUNNING;
            if (enable_thread_log) {
                std::cout << "Thread " << _name << " is started." << std::endl;
            }
        }
    }

    void stop() {
        if (_status == RUNNING) {
            // we should ensure it was running, because we can't cancel it twice
            pthread_cancel(_tid);
            _status = STOPPED;
        }
        if (enable_thread_log) {
            std::cout << "Thread " << _name << " is stopped." << std::endl;
        }
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
    pid_t _lwpid; // Lightweight Process ID
    std::string _name;
    bool _joinable;
    ThreadState _status;
};
