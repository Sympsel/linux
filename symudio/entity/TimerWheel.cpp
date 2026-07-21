#include "TimerWheel.h"

#include "EventLoop.h"

TimerWheel::TimerWheel(EventLoop *loop)
    : _tick()
      , _maxDelay(60)
      , _wheel(_maxDelay)
      , _loop(loop)
      , _timerChannel(std::make_unique<Channel>(_loop->getPoller(), createTimerFd())) {
    _timerChannel->setReadableCb([this] {
        onTimeOut();
    });
    _timerChannel->setReadListen(true);
}

void TimerWheel::addTimerTask(uint64_t id, const uint32_t delay, const TaskFunc &taskCb) {
    _loop->runInLoop([=, this] {
        registerTask(id, delay, taskCb);
    });
}

void TimerWheel::refreshTimerTask(uint64_t id) {
    _loop->runInLoop([=, this] {
        refreshTask(id);
    });
}

void TimerWheel::cancelTimerTask(uint64_t id) {
    _loop->runInLoop([=, this] {
        cancelTask(id);
    });
}
