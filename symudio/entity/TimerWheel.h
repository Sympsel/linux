#pragma once

#include <cstdint>
#include <sys/timerfd.h>
#include <memory>
#include <functional>

#include "Channel.h"
#include "Log.hpp"

class EventLoop;

using TaskFunc = std::function<void()>;

class TimerTask {
    using ReleaseFunc = std::function<void()>;

private:
    bool _canceled;
    uint64_t _id;
    // 定时任务超时时间（超时执行）
    uint32_t _delay;
    TaskFunc _taskCb;
    // 用于删除，TimerWheel中保存的定时器信息
    ReleaseFunc _releaseCb;

public:
    TimerTask(const uint64_t id, const uint32_t delay, TaskFunc taskCb)
        : _canceled(), _id(id), _delay(delay), _taskCb(std::move(taskCb)) {
    }

    ~TimerTask() {
        if (_taskCb && !_canceled) {
            _taskCb();
        }
        if (_releaseCb) {
            _releaseCb();
        }
    }

    void cancel() {
        _canceled = true;
    }

    void setRelease(ReleaseFunc releaseCb) {
        _releaseCb = std::move(releaseCb);
    }

    [[nodiscard]] uint32_t getDelay() const {
        return _delay;
    }
};

class TimerWheel {
    using TimerTaskShared = std::shared_ptr<TimerTask>;
    using TimerTaskWeak = std::weak_ptr<TimerTask>;

private:
    void removeTimer(const uint64_t id) {
        const auto it = _timers.find(id);
        if (it != _timers.end()) {
            _timers.erase(it);
        }
    }

    static int createTimerFd() {
        // CLOCK_MONOTONIC 获取时间戳
        int timerFd = timerfd_create(CLOCK_MONOTONIC, 0);
        if (timerFd < 0) {
            LOG_FATAL() << "TimerFd Create Failed!";
            exit(EXIT_FAILURE);
        }
        itimerspec timer{};
        timer.it_value.tv_sec = 1;
        timer.it_interval.tv_sec = 1;
        timerfd_settime(timerFd, 0, &timer, nullptr);
        return timerFd;
    }

    ssize_t readTimerFd() {
        // 超时次数
        uint64_t times;
        ssize_t ret = ::read(_timerChannel->fd(), &times, sizeof times);
        if (ret < 0) {
            LOG_FATAL() << "TimerFd Read Failed!";
            exit(EXIT_FAILURE);
        }
        return ret;
    }

    void onTimeOut() {
        ssize_t times = readTimerFd();
        // 超时多少次，事件轮就前进多少步
        while (times--) {
            nextStep();
        }
    }

    void registerTask(uint64_t id, const uint32_t delay, const TaskFunc &taskCb) {
        TimerTaskShared timerTask(new TimerTask(id, delay, taskCb));
        timerTask->setRelease([this, id] {
            removeTimer(id);
        });
        _timers[id] = TimerTaskWeak(timerTask);
        _wheel[(_tick + delay) % _maxDelay].emplace_back(timerTask);
    }


    // 刷新 / 延迟定时任务
    void refreshTask(uint64_t id) {
        const auto it = _timers.find(id);
        if (it != _timers.end()) {
            // lock 将 weak_ptr 提升为 shared_ptr
            TimerTaskShared timerTaskShared = it->second.lock();
            const uint32_t delay = timerTaskShared->getDelay();
            _wheel[(_tick + delay) % _maxDelay].emplace_back(timerTaskShared);
        }
    }

    void cancelTask(const uint64_t id) {
        const auto it = _timers.find(id);
        if (it != _timers.end()) {
            it->second.lock()->cancel();
        }
    }

public:
    explicit TimerWheel(EventLoop *loop);

    void addTimerTask(uint64_t id, uint32_t delay, const TaskFunc &taskCb);

    void refreshTimerTask(uint64_t id);

    void cancelTimerTask(uint64_t id);

    /**
     *
     * @warning 可能存在线程安全问题，只能在 EventLoop 中调用
     */
    bool hasTimer(uint64_t id) {
        return _timers.contains(id);
    }

    // 前进一步
    void nextStep() {
        _tick = (_tick + 1) % _maxDelay;
        // 释放所有 shared_ptr，即执行任务
        _wheel[_tick].clear();
    }

private:
    int _tick;
    int _maxDelay;
    std::vector<std::vector<TimerTaskShared> > _wheel;
    std::unordered_map<uint64_t, TimerTaskWeak> _timers;

    EventLoop *_loop;
    std::unique_ptr<Channel> _timerChannel;
};
