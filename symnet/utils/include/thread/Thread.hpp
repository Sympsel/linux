#pragma once

#include <sys/syscall.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <utility>

#include "../utils/Conf.hpp"

using func_t = std::function<void()>;

/**
 * @brief Thread wrapper class for simplified thread management.
 *
 * Provides an object-oriented interface for creating, starting,
 * stopping, and managing POSIX threads with additional metadata tracking.
 */
class Thread {
private:
    enum ThreadState {
        NEW, ///< Thread created but not started
        RUNNING, ///< Thread is currently executing
        STOPPED ///< Thread has finished or been stopped
    };

    /**
     * @brief Retrieves and stores the process ID.
     */
    void GetPid() {
        _pid = getpid();
    }

    /**
     * @brief Retrieves and stores the lightweight process ID (thread ID).
     */
    void SetLightWightPid() {
        _lwpid = static_cast<pid_t>(syscall(SYS_gettid));
    }

    /**
     * @brief Static entry point for thread execution.
     * @param args Pointer to the Thread instance
     * @return nullptr
     */
    static void *routine(void *args) {
        auto *t = static_cast<Thread *>(args);
        t->GetPid();
        t->SetLightWightPid();
        t->_func();
        t->_status = STOPPED;
        return nullptr;
    }

public:
    /**
     * @brief Constructs a Thread object with a function to execute.
     * @param f Function object to be executed in the new thread
     */
    explicit Thread(func_t f) : _func(std::move(f)), _tid(0), _pid(0), _lwpid(0), _joinable(true), _status(NEW) {
        static int thread_counter = Conf::thread_name_counter_start;
        _name = "Thread-" + std::to_string(++thread_counter);
    }

    /**
     * @brief Starts the thread execution.
     * @note Can only be called once when the thread is in NEW state.
     */
    void start() {
        if (_status == NEW) {
            pthread_create(&_tid, nullptr, routine, this);
            _status = RUNNING;
            if (Conf::thread_enable_log) {
                std::cout << "Thread " << _name << " is started." << std::endl;
            }
        }
    }

    /**
     * @brief Stops the thread by sending a cancellation request.
     * @note Only effective if the thread is in RUNNING state.
     */
    void stop() {
        if (_status == RUNNING) {
            // we should ensure it was running, because we can't cancel it twice
            pthread_cancel(_tid);
            _status = STOPPED;
        }
        if (Conf::thread_enable_log) {
            std::cout << "Thread " << _name << " is stopped." << std::endl;
        }
    }

    /**
     * @brief Detaches the thread, allowing it to run independently.
     * @note After detaching, join() cannot be called on this thread.
     */
    void detach() {
        if (_joinable && _status == RUNNING) {
            _joinable = false;
            pthread_detach(_tid);
        }
    }

    /**
     * @brief Blocks the calling thread until this thread completes.
     * @note Can only be called once on joinable threads in RUNNING state.
     */
    void join() {
        if (_joinable && _status == RUNNING) {
            pthread_join(_tid, nullptr);
            _joinable = false;
        }
    }

    /**
     * @brief Returns the human-readable thread name.
     * @return The thread name
     */
    [[nodiscard]] const std::string& GetName() const {
        return _name;
    }

    ~Thread() = default;

private:
    func_t _func; ///< Function to execute in the thread
    pthread_t _tid; ///< Thread identifier
    pid_t _pid; ///< Process ID
    pid_t _lwpid; ///< Lightweight Process ID (system thread ID)
    std::string _name; ///< Human-readable thread name
    bool _joinable; ///< Whether the thread can be joined
    ThreadState _status; ///< Current thread state
};
