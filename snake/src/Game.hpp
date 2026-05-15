#pragma once

#include "Frame.hpp"
#include <chrono>
#include <thread>
#include "Conf.hpp"

namespace Sym {
    class Game {
    private:
        void Init() {
#ifdef W
            setlocale(LC_ALL, "");
#endif
            // ncurses 初始化
            initscr(); // 初始化 ncurses
            cbreak(); // 行缓冲关闭，Ctrl+C 等信号仍可传递
            noecho(); // 禁止回显
            curs_set(0); // 隐藏光标
            keypad(stdscr, TRUE);
            nodelay(stdscr, TRUE);

            // 初始化颜色
            if (has_colors()) {
                start_color();
                init_pair(1, COLOR_GREEN, COLOR_BLACK); // 蛇身
                init_pair(2, COLOR_RED, COLOR_BLACK); // 食物
                init_pair(3, COLOR_YELLOW, COLOR_BLACK); // 边框
            }

            _frame.Init();
            _last_update_time = std::chrono::steady_clock::now();

            // 初始化资源路径
            try {
                _assets_file_path = std::filesystem::canonical(
                    std::filesystem::current_path() / "assets"
                );
            } catch (const std::exception &e) {
                LOG_WARN() << "Failed to initialize assets path: " << e.what();
                return;
            }
            {
                // 播放背景音乐
                PlaySound("游戏主旋律.mp3", true);
            }
        }

        void HandleInput() {
            const int ch = getch();
            auto &snake = _frame.GetSnake();
            switch (ch) {
                case KEY_UP:
                    if (snake.GetDirect() != DOWN) {
                        snake.SetDirect(UP);
                    }
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case KEY_DOWN:
                    if (snake.GetDirect() != UP) {
                        snake.SetDirect(DOWN);
                    }
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case KEY_LEFT:
                    if (snake.GetDirect() != RIGHT) {
                        snake.SetDirect(LEFT);
                    }
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case KEY_RIGHT:
                    if (snake.GetDirect() != LEFT) {
                        snake.SetDirect(RIGHT);
                    }
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case ' ':
                    if (_status == PAUSE) {
                        _status = RUNNING;
                    } else {
                        _status = PAUSE;
                    }
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case 'a':
                case 'A':
                    std::cout << "Speed up!" << std::endl;
                    _frame.GetSnake().Accelerate(1);
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case 's':
                case 'S':
                    std::cout << "Slow down!" << std::endl;
                    _frame.GetSnake().Accelerate(-1);
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case 'q':
                case 'Q':
                    _status = GAME_OVER;
                    break;
                default:
                    break;
            }
        }

        static void StopBackgroundMusic() {
            // 只停止循环播放的背景音乐（通过查找带 -Z 参数的进程）
            system("pkill -f 'mpg123.*-Z' 2>/dev/null");
        }

    public:
        enum Status {
            RUNNING,
            PAUSE,
            GAME_OVER
        };

        explicit Game(const int width = conf["width"],
                      const int height = conf["height"],
                      const int def_len = conf["def_len"])
            : _frame(width, height, def_len),
              _status(RUNNING),
              _score() {
        }

        void Update() {
            _score += _frame.Update();
            if (const auto &snake_status = _frame.GetSnake().GetStatus();
                snake_status == Snake::KILL_BY_SELF || snake_status == Snake::KILL_BY_WALL) {
                _status = GAME_OVER;
            }
            // todo 对死因分别处理
            ++_score;
        }

        void Run() {
            Init();
            while (_status != GAME_OVER) {
                HandleInput();
                if (_status == PAUSE) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }

                // 计算时间差
                auto currentTime = std::chrono::steady_clock::now();
                const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    currentTime - _last_update_time
                ).count();

                // 如果经过时间差大于等于移动间隔，则更新游戏状态
                if (elapsed >= _frame.GetSnake().GetMoveInterval()) {
                    Update();
#ifdef W
                    _frame.RenderW();
#else
                    _frame.Render();
#endif
                    _last_update_time = currentTime;
                }

                // 当食物过期时，设置新的食物
                if (_frame.IsFoodExpired()) {
                    _frame.SetFood();
#ifdef W
                    _frame.RenderW();
#else
                    _frame.Render();
#endif
                }

                // 休眠16ms 约60FPS, 避免CPU占用过高
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
            if (_status == GAME_OVER) {
                {
                    PlaySound("摔门声.mp3");
                    StopBackgroundMusic();
                    PlaySound("死亡旋律.mp3", true);
                }
                // todo 显示结束画面
                std::cout << "Game Over! Final Score: " << _score << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
                std::cout << "Press any key to exit..." << std::endl;
                getchar();
                // todo 提示是否重新开始
            }

            // todo play game over sound
        }

        /**
         *
         * @param sound_file 音乐文件名
         * @param is_background 是否后台播放
         */
        void PlaySound(const std::string &sound_file, bool is_background = false) const {
            if (const std::string sound_file_path = _assets_file_path + "/" + sound_file;
                std::filesystem::exists(sound_file_path)) {
                std::thread sound_thread([sound_file_path, is_background]() {
                    if (is_background) {
                        // 后台音乐：循环播放，使用 -Z 参数
                        const std::string cmd = "mpg123 -q -Z \"" + sound_file_path + "\" >/dev/null 2>&1 &";
                        system(cmd.c_str());
                    } else {
                        // 音效：只播放一次
                        const std::string cmd = "mpg123 -q \"" + sound_file_path + "\" >/dev/null 2>&1 &";
                        system(cmd.c_str());
                    }
                });
                sound_thread.detach();
            } else {
                LOG_WARN() << "Sound file not found: " << sound_file_path;
            }
        }

        ~Game() {
            endwin();
        }

    private:
        Frame _frame;
        Status _status;
        int _score;
        std::chrono::steady_clock::time_point _last_update_time;
        std::string _assets_file_path;
    };
}
