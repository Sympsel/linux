#pragma once

#include <algorithm>
#include <unordered_map>
#include <utility>

#include "Snake.hpp"

namespace Sym {
    struct Food {
        int score;
        int percent; // from 0 to 100 百分位数

        Food() : score(0), percent(0) {
        }

        Food(const int score, const int weight) : score(score), percent(weight) {
        }
    };

    class Frame {
    public:
        explicit Frame(const int width, const int height, const int def_len)
            : _width(width), _height(height),
              _curr_food_type_id(1),
              _snake(def_len),
              _rng(std::mt19937(std::random_device{}())) {
        }

        void Init() {
            {
                _foods[1] = {10, 80};
                _foods[2] = {20, 100};
            }
            _snake.Init();
            SetFood();
        }

        /**
         * @brief 更新游戏状态
         * @return 蛇吃食物的得分
         */
        int Update() {
            _snake.move();

            if (_snake.GetHead()->pos == _food_pos) {
                _snake.Grow();
                const int score = _foods[_curr_food_type_id].score;
                SetFood();
                return score;
            }

            if (auto snake_body = _snake.GetBodyPos();
                (std::find(snake_body.begin() + 1, snake_body.end(),
                           _snake.GetHead()->pos) != snake_body.end())) {
                _snake.SetStatus(Snake::KILL_BY_SELF);
            } else if (_snake.GetHead()->pos.first <= 0 || _snake.GetHead()->pos.first >= _width ||
                       _snake.GetHead()->pos.second <= 0 || _snake.GetHead()->pos.second >= _height) {
                _snake.SetStatus(Snake::KILL_BY_WALL);
            }
            return 0;
        }

        void SetFood() {
            std::uniform_int_distribution<int> width_dist(1, _width - 1);
            std::uniform_int_distribution<int> height_dist(1, _height - 1);

            // todo _foods may be empty
            std::uniform_int_distribution<int> type_dist(0, static_cast<int>(_foods.size()) - 1);
            const int type_id = type_dist(_rng);
            for (auto [id, food]: _foods) {
                if (food.percent > type_id) {
                    _curr_food_type_id = id;
                    break;
                }
            }

            bool flag = true;
            int x, y;
            while (flag) {
                flag = false;
                x = width_dist(_rng);
                y = height_dist(_rng);
                if (const auto &body = _snake.GetBodyPos();
                    std::find(body.begin(), body.end(), Pos{x, y}) != body.end()) {
                    flag = true;
                }
            }

            _food_pos = {x, y};
        }

        [[nodiscard]] const Pos &GetFoodPos() const {
            return _food_pos;
        }

        [[nodiscard]] Snake &GetSnake() {
            return _snake;
        }


        void Render() const {
            clear();
            // 绘制边框
            attron(COLOR_PAIR(3));
            for (int i = 0; i <= _width + 1; ++i) {
                mvaddch(0, i, ACS_HLINE);
                mvaddch(_height + 1, i, ACS_HLINE);
            }
            for (int i = 0; i <= _height + 1; ++i) {
                mvaddch(i, 0, ACS_VLINE);
                mvaddch(i, _width + 1, ACS_VLINE);
            }
            // 绘制边框角
            mvaddch(0, 0, ACS_ULCORNER);
            mvaddch(0, _width + 1, ACS_URCORNER);
            mvaddch(_height + 1, 0, ACS_LLCORNER);
            mvaddch(_height + 1, _width + 1, ACS_LRCORNER);
            attroff(COLOR_PAIR(3));

            // 绘制食物
            attron(COLOR_PAIR(2));
            mvaddch(_food_pos.second + 1, _food_pos.first + 1, '*');
            attroff(COLOR_PAIR(2));

            // 绘制蛇
            const auto body = _snake.GetBodyPos();
            attron(COLOR_PAIR(1));
            for (size_t i = 0; i < body.size(); i++) {
                const char c = (i == 0) ? '@' : 'o';
                mvaddch(body[i].second + 1, body[i].first + 1, c);
            }
            attroff(COLOR_PAIR(1));

            // 刷新屏幕
            refresh();
        }

        [[nodiscard]] const Snake& GetSnake() const {
            return _snake;
        }

        ~Frame() = default;

    private:
        int _width;
        int _height;
        std::unordered_map<int, Food> _foods;
        Pos _food_pos;
        int _curr_food_type_id;
        Snake _snake;
        std::mt19937 _rng;
    };
}
