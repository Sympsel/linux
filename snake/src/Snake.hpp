#pragma once

#include <utility>
#include <random>
#include <ncurses.h>
#include "Conf.hpp"

namespace Sym {
    using Pos = std::pair<int, int>;
    using SpeedLevel = int;

    enum Direct {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    static constexpr std::pair<int, int> DIR_OFFSET[4] = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0}
    };

    enum NodeType {
        BODY,
        HEAD
    };

    struct Node {
        Pos pos;
        NodeType type;
        Node *prev;
        Node *next;
    };

    class Snake {
    public:
        enum class Status {
            NORMAL,
            FACE_BODY,
            FACE_WALL,
            FACE_FOOD
        };

    public:
        explicit Snake(const int def_len)
            : _head(nullptr), _tail(nullptr),
              _direct(RIGHT),
              _len(def_len),
              _status(Status::NORMAL),
              _move_interval(200),
              _speed_level(conf["def_speed_level"]) {
        }

        void SetDirect(const Direct direct) {
            if (
                (direct == UP && _direct != DOWN) ||
                (direct == DOWN && _direct != UP) ||
                (direct == LEFT && _direct != RIGHT) ||
                (direct == RIGHT && _direct != LEFT)
            ) {
                _direct = direct;
            }
        }

        void SetMoveSpeed(SpeedLevel level) {
            const int max_speed_level = conf["max_speed_level"];
            if (level < 1) level = 1;
            else if (level > max_speed_level) level = max_speed_level;
            else {
            }
            _speed_level = level;
            const int offset = level - (max_speed_level + 1) / 2;
            _move_interval = std::max(conf["move_interval"] - offset * 30, conf["min_move_interval"]);
        }

        [[nodiscard]] SpeedLevel GetMoveLevel() const {
            return _speed_level;
        }

        void Accelerate(const int modify) {
            SetMoveSpeed(_speed_level + modify);
        }

        [[nodiscard]] int GetDirect() const {
            return _direct;
        }

        [[nodiscard]] Node *GetHead() const {
            return _head;
        }

        [[nodiscard]] std::vector<Pos> GetBodyPos() const {
            std::vector<Pos> pos;
            const Node *cur = _head;
            while (cur) {
                pos.emplace_back(cur->pos);
                cur = cur->next;
            }
            return pos;
        }

        void Grow() {
            Pos pos = _head->pos;
            pos.first += DIR_OFFSET[_direct].first;
            pos.second += DIR_OFFSET[_direct].second;

            const auto newNode = new Node{
                pos, HEAD, nullptr, _head
            };

            _head->prev = newNode;
            _head = newNode;
        }

        void Init() {
            _tail = new Node{{5, 4}, BODY, nullptr, nullptr};
            _head = new Node{{5, 5}, HEAD, nullptr, nullptr};
            _tail->prev = _head;
            _head->next = _tail;

            int cnt = conf["def_len"] - 2;
            while (cnt--) {
                Grow();
            }
        }

        void move() {
            // 根据方向计算新头部位置
            Pos newHeadPos = _head->pos;
            newHeadPos.first += DIR_OFFSET[_direct].first;
            newHeadPos.second += DIR_OFFSET[_direct].second;

            // 将尾部移动到头部前方
            _tail->pos = newHeadPos;
            _tail->type = HEAD;
            _head->type = BODY;

            // 调整链表：将尾部节点移到头部
            Node *temp = _tail->prev;
            _tail->prev->next = nullptr;

            _tail->next = _head;
            _head->prev = _tail;
            _head = _tail;
            _tail = temp;
        }

        [[nodiscard]] int GetMoveInterval() const {
            return _move_interval;
        }

        void SetStatus(const Status status) {
            _status = status;
        }

        [[nodiscard]] const Status &GetStatus() const {
            return _status;
        }

        ~Snake() {
            const Node *cur = _head;
            while (cur) {
                const Node *next = cur->next;
                delete cur;
                cur = next;
            }
        }

    private:
        Node *_head;
        Node *_tail;
        Direct _direct;
        int _len;
        Status _status;
        int _move_interval;
        int _speed_level;
    };
}
