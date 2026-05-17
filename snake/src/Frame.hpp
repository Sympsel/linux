#pragma once

#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <utility>

#include "Snake.hpp"
#include "Conf.hpp"

namespace Sym {
    struct Food;

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
                int id = 0;
                const auto &foods = conf.GetConf().food_list;
                for (const auto &food: foods) {
                    _foods[id] = food;
                    _foods[id].percent += foods[id].percent;
                    ++id;
                }
            }
            _snake.Init();
            SetFood();
        }

        /**
         * @brief 更新游戏状态
         * @return 蛇吃食物的得分
         */
        int Update() {
            while (true) {
                switch (_snake.GetStatus()) {
                    case Snake::Status::NORMAL:
                        _snake.move();
                        if (_snake.GetHead()->pos == _food_pos) {
                            _snake.SetStatus(Snake::Status::FACE_FOOD);
                        } else if (auto snake_body = _snake.GetBodyPos();
                            std::find(snake_body.begin() + 1, snake_body.end(),
                                      _snake.GetHead()->pos) != snake_body.end()) {
                            _snake.SetStatus(Snake::Status::FACE_BODY);
                        } else if (_snake.GetHead()->pos.first < 0 || _snake.GetHead()->pos.first >= _width ||
                                   _snake.GetHead()->pos.second < 1 || _snake.GetHead()->pos.second >= _height) {
                            _snake.SetStatus(Snake::Status::FACE_WALL);
                        }
                        break;
                    case Snake::Status::FACE_BODY:
                    case Snake::Status::FACE_WALL:
                        return 0;
                    case Snake::Status::FACE_FOOD:
                        _snake.Grow();
                        const int score = _foods[_curr_food_type_id].score;
                        SetFood();
                        // 重新设置回正常状态
                        _snake.SetStatus(Snake::Status::NORMAL);
                        return score;
                }
                return 0;
            }
        }

        void SetFood() {
            std::uniform_int_distribution<int> width_dist(1, _width - 1);
            std::uniform_int_distribution<int> height_dist(2, _height - 1);
            std::uniform_int_distribution<int> type_dist(1, 100);
            const int type_id = type_dist(_rng);
            if (_foods.empty()) {
                LOG_FATAL() << "Food list is empty!";
                sleep(2);
                exit(EXIT_FAILURE);
            }

            // 遍历累加后的百分位数，找到第一个大于随机数的食物类型
            for (const auto &[id, food]: _foods) {
                if (food.percent >= type_id) {
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
                const auto &body = _snake.GetBodyPos();
                for (const auto &pos: body) {
                    if (pos == Pos{x, y}) {
                        flag = true;
                        break;
                    }
                }
            }

            _food_pos = {x, y};
            _food_last_time = std::chrono::steady_clock::now();
        }

        [[nodiscard]] const Pos &GetFoodPos() const {
            return _food_pos;
        }

        [[nodiscard]] Snake &GetSnake() {
            return _snake;
        }


        /**
         * @brief 绘制游戏画面, 宽体字符绘制
         */
        void RenderW() const {
            clear();

            attron(COLOR_PAIR(3));
            // 宽字符模式下，宽度需要 * 2
            for (int i = 0; i <= _width * 2 + 1; ++i) {
                mvaddch(1, i, ACS_HLINE);
                mvaddch(_height + 1, i, ACS_HLINE);
            }
            for (int i = 0; i <= _height + 1; ++i) {
                mvaddch(i, 0, ACS_VLINE);
                mvaddch(i, _width * 2 + 1, ACS_VLINE);
            }
            mvaddch(1, 0, ACS_ULCORNER);
            mvaddch(1, _width * 2 + 1, ACS_URCORNER);
            mvaddch(_height + 1, 0, ACS_LLCORNER);
            mvaddch(_height + 1, _width * 2 + 1, ACS_LRCORNER);
            attroff(COLOR_PAIR(3));

            // 开启颜色
            attron(COLOR_PAIR(2));
            // 使用 mvprintw 直接输出 UTF-8 字符串
            mvprintw(_food_pos.second + 1, _food_pos.first * 2 + 1, "%s",
                     _foods.at(_curr_food_type_id).signal.c_str());
            attroff(COLOR_PAIR(2));

            const auto body = _snake.GetBodyPos();
            attron(COLOR_PAIR(1));
            for (size_t i = 0; i < body.size(); i++) {
                if (i == 0) {
                    mvprintw(body[i].second + 1, body[i].first * 2 + 1, "●");
                } else {
                    mvprintw(body[i].second + 1, body[i].first * 2 + 1, "○");
                }
            }
            attroff(COLOR_PAIR(1));

            refresh();
        }

        /**
         * @brief 绘制游戏画面, 字符绘制
         */
        void Render() const {
            clear();
            attron(COLOR_PAIR(3));
            for (int i = 0; i <= _width + 1; ++i) {
                mvaddch(0, i, ACS_HLINE);
                mvaddch(_height + 1, i, ACS_HLINE);
            }
            for (int i = 0; i <= _height + 1; ++i) {
                mvaddch(i, 0, ACS_VLINE);
                mvaddch(i, _width + 1, ACS_VLINE);
            }
            mvaddch(0, 0, ACS_ULCORNER);
            mvaddch(0, _width + 1, ACS_URCORNER);
            mvaddch(_height + 1, 0, ACS_LLCORNER);
            mvaddch(_height + 1, _width + 1, ACS_LRCORNER);
            attroff(COLOR_PAIR(3));

            attron(COLOR_PAIR(2));

            mvprintw(_food_pos.second + 1, _food_pos.first + 1, "%s",
                     _foods.at(_curr_food_type_id).signal.c_str());
            attroff(COLOR_PAIR(2));

            const auto body = _snake.GetBodyPos();
            attron(COLOR_PAIR(1));
            for (size_t i = 0; i < body.size(); i++) {
                const char c = (i == 0) ? '@' : 'o';
                mvaddch(body[i].second + 1, body[i].first + 1, c);
            }
            attroff(COLOR_PAIR(1));

            refresh();
        }

        [[nodiscard]] const Snake &GetSnake() const {
            return _snake;
        }

        [[nodiscard]] bool IsFoodExpired() const {
            const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - _food_last_time
            ).count();
            return elapsed > _foods.at(_curr_food_type_id).duration_time;
        }

        [[nodiscard]] int GetHeight() const {
            return _height;
        }

        ~Frame() = default;

    private:
        int _width;
        int _height;
        std::unordered_map<int, Food> _foods;
        Pos _food_pos;
        int _curr_food_type_id;
        std::chrono::steady_clock::time_point _food_last_time;
        Snake _snake;
        std::mt19937 _rng;
    };
}
