#ifndef _THREAD_HPP_
#define _THREAD_HPP_

#include <iostream>
#include <functional>
#include <unistd.h>
#include <sys/syscall.h>

using func_t = std::function<void()>;

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
        return nullptr;
    }
   public:
    Thread(func_t f) : _func(f), _joinabled(true) {
        _name = "Thread-" + std::to_string(_tid);
    }

    void start() {
        int n = pthread_create(&_tid, nullptr, routine, this);
    }

    void stop() {
        pthread_cancel(_tid);
        std::cout << "Thread " << _name << " is stopped." << std::endl;
    }

    void detach() {
        if (_joinabled) {
            _joinabled = false;
            pthread_detach(_tid);
        }
    }

    void join() {
        if (_joinabled) {
            pthread_join(_tid, nullptr);
            _joinabled = false;
        }
    }

    ~Thread() {}

   private:
    pthread_t _tid;
    pid_t _pid;
    pid_t _lwpid;
    std::string _name;
    bool _joinabled;

    func_t _func;
};

#endif  // _THREAD_HPP_