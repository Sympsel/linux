#include "Channel.h"

#include "Poller.h"

// 移除所有事件监控
void Channel::remove() {
    return _poller->removeEvent(this);
}

bool Channel::update() {
    return _poller->updateEvent(this);
}

void Channel::handleEvent() {
    if (_revents & EPOLLIN
        || _revents & EPOLLRDHUP /* 表示对端关闭 */
        || _revents & EPOLLPRI /* 表示对端发送紧急数据 */) {
        if (_readCb) {
            _readCb();
        }
        if (_anyEventCb) {
            _anyEventCb();
        }
        }
    // 有可能会释放连接的操作，一次只处理一个
    if (_revents & EPOLLOUT) {
        if (_writeCb) {
            _writeCb();
        }
        if (_anyEventCb) {
            _anyEventCb();
        }
    } else if (_revents & EPOLLERR) {
        if (_anyEventCb) {
            _anyEventCb();
        }
        if (_errorCb) {
            _errorCb();
        }
        if (_closeCb) {
            _closeCb();
        }
    } else if (_revents & EPOLLHUP/* 表示对端关闭 */) {
        if (_anyEventCb) {
            _anyEventCb();
        }
        if (_closeCb) {
            _closeCb();
        }
    }
}
