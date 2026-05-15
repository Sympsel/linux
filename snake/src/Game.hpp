#pragma once

#include "Frame.hpp"
#include <chrono>
#include <thread>
#include "Conf.hpp"
#include "Log.hpp"

namespace Sym {
    class Game {
    private:
        void Init() {
            LOG_INFO() << "Initializing Game...";
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
                LOG_DEBUG() << "Color support initialized";
            } else {
                LOG_WARN() << "Terminal does not support color";
            }

            _frame.Init();
            _last_update_time = std::chrono::steady_clock::now();

            // 初始化资源路径
            try {
                _assets_file_path = std::filesystem::canonical(
                    std::filesystem::current_path() / "assets"
                );
                LOG_INFO() << "Assets path initialized: " << _assets_file_path;
            } catch (const std::exception &e) {
                LOG_WARN() << "Failed to initialize assets path: " << e.what();
                return;
            }
            {
                if (const std::string bg_music = "游戏主旋律.mp3";
                    std::filesystem::exists(_assets_file_path + "/" + bg_music)) {
                    PlaySound(bg_music, true);
                    LOG_INFO() << "Background music started: " << bg_music;
                } else {
                    LOG_WARN() << "Background music file not found: " << bg_music;
                }
            }
        }

        void HandleInput() {
            const int ch = getch();
            auto &snake = _frame.GetSnake();
            switch (ch) {
                case KEY_UP:
                    if (snake.GetDirect() != DOWN) {
                        snake.SetDirect(UP);
                        LOG_DEBUG() << "Key pressed: Up";
                    }
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case KEY_DOWN:
                    if (snake.GetDirect() != UP) {
                        snake.SetDirect(DOWN);
                        LOG_DEBUG() << "Key pressed: Down";
                    }
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case KEY_LEFT:
                    if (snake.GetDirect() != RIGHT) {
                        snake.SetDirect(LEFT);
                        LOG_DEBUG() << "Key pressed: Left";
                    }
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case KEY_RIGHT:
                    if (snake.GetDirect() != LEFT) {
                        snake.SetDirect(RIGHT);
                        LOG_DEBUG() << "Key pressed: Right";
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
                    LOG_DEBUG() << "Pressed: Pause";
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case 'a':
                case 'A':
                    RenderBottomTipsBar("Speed up!");
                    LOG_DEBUG() << "Pressed: Speed up";
                    _frame.GetSnake().Accelerate(1);
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case 's':
                case 'S':
                    RenderBottomTipsBar("Slow down!");
                    LOG_DEBUG() << "Pressed: Slow down";
                    _frame.GetSnake().Accelerate(-1);
                    {
                        PlaySound("清脆响声.mp3");
                    }
                    break;
                case 'q':
                case 'Q':
                    LOG_INFO() << "User requested quit (pressed Q)";

                    _status = GAME_OVER;
                    break;
                default:
                    break;
            }
        }

        static void StopAllSounds() {
            // 停止所有 mpg123 进程（包括背景音乐和音效）
            system("pkill -f mpg123 2>/dev/null");
        }

        static void StopBackgroundMusic() {
            // 只停止循环播放的背景音乐（通过查找带 -Z 参数的进程）
            system("pkill -f 'mpg123.*-Z' 2>/dev/null");
        }

        void RenderTopStatusBar() const {
            // 在顶部边框上方显示状态栏（第0行）
            attron(COLOR_PAIR(3) | A_BOLD);

            // 显示分数
            mvprintw(0, 2, " Score: %-6d ", _score);

            // 显示速度等级
            const int speed_level = _frame.GetSnake().GetMoveLevel();
            mvprintw(0, 20, " Speed: %-3d ", speed_level);

            // 显示蛇长度
            const int snake_length = static_cast<int>(_frame.GetSnake().GetBodyPos().size());
            mvprintw(0, 35, " Length: %-4d ", snake_length);

            // 显示游戏状态
            const char *status_text = (_status == RUNNING) ? "RUNNING" : (_status == PAUSE) ? "PAUSED" : "GAME OVER";
            mvprintw(0, 55, " Status: %-10s ", status_text);

            attroff(COLOR_PAIR(3) | A_BOLD);
        }

        void RenderBottomTipsBar(const std::string &tips, const int offset = 0) const {
            attron(COLOR_PAIR(3) | A_BOLD);

            const int row_begin = _frame.GetHeight() + 2;
            mvprintw(row_begin + offset, 2, "%s", tips.c_str());

            attroff(COLOR_PAIR(3) | A_BOLD);
        }

        void Update() {
            _score += _frame.Update();
            const auto &snake_status = _frame.GetSnake().GetStatus();
            if (snake_status == Snake::KILL_BY_SELF ||
                snake_status == Snake::KILL_BY_WALL) {
                _status = GAME_OVER;
            }
            if (snake_status == Snake::KILL_BY_SELF) {
                LOG_WARN() << "Snake killed itself! Final score: " << _score;
            } else if (snake_status == Snake::KILL_BY_WALL) {
                LOG_WARN() << "Snake hit the wall! Final score: " << _score;
            } else {
            }
            // todo 对死因分别处理
            ++_score;
            RenderTopStatusBar();
            refresh();
        }

        /**
         *
         * @param sound_file 音乐文件名
         * @param is_background 是否后台播放
         */
        void PlaySound(const std::string &sound_file, bool is_background = false) const {
            if (const std::string sound_file_path = _assets_file_path + "/" + sound_file;
                std::filesystem::exists(sound_file_path)) {
                std::thread sound_thread([sound_file_path, is_background,sound_file]() {
                    if (is_background) {
                        // 后台音乐：循环播放，使用 -Z 参数
                        const std::string cmd = "mpg123 -q -Z \"" + sound_file_path + "\" >/dev/null 2>&1 &";
                        system(cmd.c_str());
                        LOG_DEBUG() << "Playing background music (loop): " << sound_file;
                    } else {
                        // 音效：只播放一次
                        const std::string cmd = "mpg123 -q \"" + sound_file_path + "\" >/dev/null 2>&1 &";
                        system(cmd.c_str());
                        LOG_DEBUG() << "Playing sound effect: " << sound_file;
                    }
                });
                sound_thread.detach();
            } else {
                LOG_WARN() << "Sound file not found: " << sound_file_path;
            }
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


        bool Run() {
            LOG_INFO() << "Starting game ...";
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
                    RenderTopStatusBar();
                    refresh();
                }

                // 当食物过期时，设置新的食物
                if (_frame.IsFoodExpired()) {
                    _frame.SetFood();
                    LOG_DEBUG() << "Food expired, new food placed at: ("
                            << _frame.GetFoodPos().first << ", "
                            << _frame.GetFoodPos().second << ")";
#ifdef W
                    _frame.RenderW();
#else
                    _frame.Render();
#endif
                    RenderTopStatusBar();
                    refresh();
                }

                // 休眠16ms 约60FPS, 避免CPU占用过高
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            if (_status == GAME_OVER) {
                LOG_INFO() << "Game Over! Final Score: " << _score;
                // 如果是按'q'退出,则不播放死亡音乐
                if (_frame.GetSnake().GetStatus() == Snake::KILL_BY_SELF ||
                    _frame.GetSnake().GetStatus() == Snake::KILL_BY_WALL) {
                    LOG_INFO() << "Playing death sound sequence";
                    PlaySound("摔门声.mp3");
                    StopBackgroundMusic();
                    PlaySound("死亡旋律.mp3", true);
                } else {
                    LOG_INFO() << "User quit without death (pressed Q)";

                    StopBackgroundMusic();
                }

                // 显示结束画面
                RenderTopStatusBar();
                RenderBottomTipsBar("GAME OVER! Final Score: " + std::to_string(_score));
                refresh();

                // 用于防止用户一直操作导致结算画面直接退出
                sleep(3);
                // 清空输入缓冲区
                while (getch() != ERR) {
                }

                RenderTopStatusBar();
                RenderBottomTipsBar("GAME OVER! Final Score: " + std::to_string(_score));
                RenderBottomTipsBar("Press 'c' to continue or any other key to quit...", 1);
                refresh();
                // 等待用户按键
                nodelay(stdscr, FALSE); // 改为阻塞模式，等待按键

                const char key = getch();
                nodelay(stdscr, TRUE); // 恢复非阻塞模式
                if (key == 'c') {
                    LOG_INFO() << "User chose to play again";
                    return true;
                }
                LOG_INFO() << "User chose to quit";
            }
            LOG_INFO() << "Game session ended";
            return false;
        }


        ~Game() {
            LOG_DEBUG() << "Destroying Game object...";
            StopAllSounds();
            endwin();
            LOG_DEBUG() << "Game object destroyed";
        }

    private:
        Frame _frame;
        Status _status;
        int _score;
        std::chrono::steady_clock::time_point _last_update_time;
        std::string _assets_file_path;
    };
}
