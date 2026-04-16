# ifndef BLOCK_QUEUE_HPP
# define BLOCK_QUEUE_HPP

#include <queue>
#include "Mutex.hpp"
#include "Cond.hpp"

static const int gdefaultcap = 5;

template<typename T>
class BlockQueue {

public:
    BlockQueue(size_t cap = gdefaultcap) : _cap(cap),
    _prod_sleeper_cnt(0), _cons_sleeper_cnt(0),
    _low_water_mark(cap / 4), _high_water_mark(cap / 4 * 3) {}
    
    ~BlockQueue() {}

    void Push(const T& item) {
        {
            LockGuard lockguard(&_mutex);
            while (_queue.size() >= _cap) {
                ++_prod_sleeper_cnt;
                _cond_prod.Wait(_mutex);
                --_prod_sleeper_cnt;
            }
            _queue.push(item);
            if (_queue.size() > _high_water_mark && _cons_sleeper_cnt > 0)
                _cond_cons.NotifyOne();
        }
    }

    void Pop(T *out) {
        {
            LockGuard lockguard(&_mutex);
            while (_queue.empty()) {
                ++_cons_sleeper_cnt;
                _cond_cons.Wait(_mutex);
                --_cons_sleeper_cnt;
            }
            *out = _queue.front();
            _queue.pop();
            if (_queue.size() < _low_water_mark && _prod_sleeper_cnt > 0)
                _cond_prod.NotifyOne();
        }
    }


private:
    std::queue<T> _queue;
    size_t _cap;
    Mutex _mutex;

    Cond _cond_cons;
    Cond _cond_prod;

    int _prod_sleeper_cnt;
    int _cons_sleeper_cnt;

    size_t _low_water_mark;
    size_t _high_water_mark;
};

#endif // BLOCK_QUEUE_HPP
