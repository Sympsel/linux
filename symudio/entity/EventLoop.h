#pragma once

#include <thread>
#include <sys/eventfd.h>

#include "Poller.h"
#include "TimerWheel.h"

class EventLoop {
private:
    using Task = std::function<void()>;

    static int createEventFd() {
        // EFD_CLOEXEC 创建的文件描述符，在 fork 后，子进程会继承该文件描述符，但是子进程会继承一个 cloexec 文件描述符
        int eventFd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if (eventFd < 0) {
            LOG_FATAL() << "EventFd Create Failed!";
            exit(EXIT_FAILURE);
        }
        return eventFd;
    }

    void readEventFd() {
        uint64_t res;
        ssize_t ret = ::read(_eventChannel->fd(), &res, sizeof res);
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                return;
            }
            LOG_ERROR() << "EventFd Read Failed!";
            exit(EXIT_FAILURE);
        }
    }

    void weakUpEventFd() {
        uint64_t val = 1;
        ssize_t ret = ::write(_eventChannel->fd(), &val, sizeof val);
        if (ret < 0) {
            if (errno == EINTR) {
                return;
            }
            LOG_ERROR() << "EventFd Write Failed!";
            exit(EXIT_FAILURE);
        }
    }

public:
    EventLoop()
        : _threadId(std::this_thread::get_id())
          , _eventChannel(std::make_unique<Channel>(&_poller, createEventFd()))
          , _timerWheel(this) {
        // 给 eventFd 添加可读事件回调。读取事件通知次数
        _eventChannel->setReadableCb([this] {
            readEventFd();
        });
        _eventChannel->setReadListen(true);
    }

    /**
     * @brief 在 EventLoop 线程中执行回调
     *
     * - 同线程调用：立即同步执行
     * - 跨线程调用：异步加入队列，通过 eventfd 唤醒 EventLoop 线程后执行
     *
     * @param cb 要执行的回调函数
     */
    void runInLoop(const Task &cb) {
        if (isInLoop()) {
            cb();
        } else {
            queueInLoop(cb);
        }
    }

    void queueInLoop(const Task &cb) {
        {
            std::unique_lock<std::mutex> _lock(_mutex);
            _tasks.emplace_back(cb);
        }
        // 唤醒有可能没有事件就绪而导致的 epoll 阻塞
        weakUpEventFd();
    }

    // 用于判断当前线程是否是 EventLoop 对应的线程
    bool isInLoop() {
        return _threadId == std::this_thread::get_id();
    }

    void updateEvent(Channel *channel) {
        _poller.updateEvent(channel);
    }

    void removeEvent(Channel *channel) {
        _poller.removeEvent(channel);
    }

    void addTimerTask(uint64_t id, uint32_t delay, const TaskFunc &cb) {
        _timerWheel.addTimerTask(id, delay, cb);
    }

    void refreshTimerTask(uint64_t id) {
        _timerWheel.refreshTimerTask(id);
    }

    void cancelTimerTask(uint64_t id) {
        _timerWheel.cancelTimerTask(id);
    }

    bool hasTimer(uint64_t id) {
        return _timerWheel.hasTimer(id);
    }

    void runAllTask() {
        std::vector<Task> tasks;
        {
            std::unique_lock<std::mutex> _lock(_mutex);
            tasks = std::exchange(tasks, _tasks);
        }
        for (auto &task: tasks) {
            task();
        }
    }

    // 事件监控 -> 就绪事件处理 -> 执行任务
    void start() {
        std::vector<Channel *> activeChannels;
        _poller.poll(activeChannels);
        // 处理 I/O 逻辑
        for (auto &channel: activeChannels) {
            channel->handleEvent();
        }
        // 处理定时任务
        runAllTask();
    }

    Poller *getPoller() {
        return &_poller;
    }

private:
    std::thread::id _threadId;
    Poller _poller;
    std::unique_ptr<Channel> _eventChannel;
    std::vector<Task> _tasks;
    std::mutex _mutex;
    TimerWheel _timerWheel;
};
