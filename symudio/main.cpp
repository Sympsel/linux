#include <functional>
#include <memory>
#include <print>
#include <regex>
#include <utility>
#include <unistd.h>
#include <sys/timerfd.h>

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
    TimerTask(uint64_t id, uint32_t delay, TaskFunc taskCb)
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

    uint32_t getDelay() const {
        return _delay;
    }
};

class TimerWheel {
    using TimerTaskShared = std::shared_ptr<TimerTask>;
    using TimerTaskWeak = std::weak_ptr<TimerTask>;

private:
    int _tick;
    int _maxDelay;
    std::vector<std::vector<TimerTaskShared> > _wheel;
    std::unordered_map<uint64_t, TimerTaskWeak> _timers;

private:
    void removeTimer(uint64_t id) {
        const auto it = _timers.find(id);
        if (it != _timers.end()) {
            _timers.erase(it);
        }
    }

public:
    TimerWheel() : _tick(), _maxDelay(60), _wheel(_maxDelay) {
    }

    void registerTask(uint64_t id, uint32_t delay, const TaskFunc &taskCb) {
        TimerTaskShared timerTaskShared(new TimerTask(id, delay, taskCb));
        timerTaskShared->setRelease([this, id] { removeTimer(id); });
        _timers[id] = TimerTaskWeak(timerTaskShared);
        _wheel[(_tick + delay) % _maxDelay].emplace_back(timerTaskShared);
    }

    // 刷新 / 延迟定时任务
    void refreshTask(uint64_t id) {
        const auto it = _timers.find(id);
        if (it != _timers.end()) {
            // lock 将 weak_ptr 提升为 shared_ptr
            TimerTaskShared timerTaskShared = it->second.lock();
            uint32_t delay = timerTaskShared->getDelay();
            _wheel[(_tick + delay) % _maxDelay].emplace_back(timerTaskShared);
        }
    }

    void cancelTask(const uint64_t id) {
        const auto it = _timers.find(id);
        if (it != _timers.end()) {
            it->second.lock()->cancel();
        }
    }

    // 每秒执行一次
    void run() {
        _tick = (_tick + 1) % _maxDelay;
        // 释放所有 shared_ptr，即执行任务
        _wheel[_tick].clear();
    }
};

class Test {
public:
    Test() {
        std::println("构造");
    }

    ~Test() {
        std::println("析构");
    }
};

void delTest(const Test *t) {
    delete t;
}

int main() {
    // std::string str = "/numbers/1234";
    // std::regex reg{"/numbers/(\\d+)"};
    // std::smatch sm;
    // bool ret = std::regex_match(str, sm, reg);
    // if (ret) {
    //     for (auto &match : sm) {
    //         std::println("{}", match.str());
    //     }
    // } else {
    //     std::println("匹配错误");
    //     return -1;
    // }
    std::string httpReq = "GET /numbers/login?user=sym&password=123456 HTTP/1.1";
    std::regex reg("(GET|HEAD|matchPOST|PUT|DELETE) ([^?]*)\\?(.*) (HTTP/1\\.[01])");
    std::smatch sm;
    bool ret = std::regex_match(httpReq, sm, reg);
    if (ret) {
        for (auto match : sm) {
            std::println("{}", match.str());
        }
    }

    return 0;
}

// int main() {
//     TimerWheel tw;
//     Test *t = new Test();
//     tw.registerTask(1, 5, [t] { delTest(t); });
//     for (int i{}; i < 5; ++i) {
//         sleep(1);
//         tw.refreshTask(1);
//         std::println("刷新了一下定时任务");
//     }
//
//     tw.cancelTask(1);
//
//     while (true) {
//         sleep(1);
//         std::println("滴答");
//         tw.run();
//     }
//     return 0;
// }