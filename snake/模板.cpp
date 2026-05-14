#include <iostream>
#include <unistd.h>
#include <ncurses.h>
#include <vector>
#include <cstdlib>
#include <ctime>

// 游戏区域大小（不含边框）
const int GAME_HEIGHT = 20;
const int GAME_WIDTH  = 60;

// 需要的最小终端尺寸（边框+状态栏各占1行）
const int NEED_ROWS = GAME_HEIGHT + 4;  // 上边框1 + 游戏20 + 下边框1 + 状态2
const int NEED_COLS = GAME_WIDTH  + 2;  // 左右边框各1

struct Pos { int y, x; };

enum Dir { UP, DOWN, LEFT, RIGHT };

int main() {
    // ========== 1. ncurses 初始化 ==========
    initscr();
    cbreak();              // 行缓冲关闭，Ctrl+C 等信号仍可传递
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    // 检查颜色支持
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // 蛇身
        init_pair(2, COLOR_RED,   COLOR_BLACK);   // 食物
        init_pair(3, COLOR_YELLOW,COLOR_BLACK);  // 边框
    }

    // ========== 2. 终端尺寸检查 ==========
    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);

    if (term_h < NEED_ROWS || term_w < NEED_COLS) {
        endwin();
        printf("Terminal too small!\n");
        printf("Need: %dx%d, Got: %dx%d\n", NEED_COLS, NEED_ROWS, term_w, term_h);
        printf("Tip: Resize terminal or run with: xterm -geometry %dx%d -e ./snake\n",
               NEED_COLS, NEED_ROWS);
        return 1;
    }

    // ========== 3. 创建独立的游戏窗口 ==========
    // 在屏幕中央偏上放置游戏区域
    int start_y = (term_h - NEED_ROWS) / 2;
    int start_x = (term_w - NEED_COLS) / 2;

    // 新建一个带边框的子窗口
    WINDOW *win = newwin(GAME_HEIGHT + 2, GAME_WIDTH + 2, start_y, start_x);
    keypad(win, TRUE);
    nodelay(win, TRUE);
    box(win, 0, 0);  // 自动使用 ACS 字符画边框

    // ========== 4. 游戏状态初始化 ==========
    std::vector<Pos> snake;
    snake.push_back({GAME_HEIGHT/2, GAME_WIDTH/2});
    snake.push_back({GAME_HEIGHT/2, GAME_WIDTH/2 - 1});
    snake.push_back({GAME_HEIGHT/2, GAME_WIDTH/2 - 2});

    Dir dir = RIGHT;
    Pos food = {rand() % GAME_HEIGHT, rand() % GAME_WIDTH};
    int score = 0;
    bool game_over = false;

    srand(time(nullptr));

    // ========== 5. 游戏主循环 ==========
    while (!game_over) {
        // --- 输入处理 ---
        int ch = getch();
        switch (ch) {
            case KEY_UP:    if (dir != DOWN)  dir = UP;    break;
            case KEY_DOWN:  if (dir != UP)    dir = DOWN;  break;
            case KEY_LEFT:  if (dir != RIGHT) dir = LEFT;  break;
            case KEY_RIGHT: if (dir != LEFT)  dir = RIGHT; break;
            case 'q':       game_over = true; break;
        }

        // --- 移动蛇头 ---
        Pos head = snake[0];
        Pos new_head = head;
        switch (dir) {
            case UP:    new_head.y--; break;
            case DOWN:  new_head.y++; break;
            case LEFT:  new_head.x--; break;
            case RIGHT: new_head.x++; break;
        }

        // --- 碰撞检测 ---
        // 撞墙
        if (new_head.y < 0 || new_head.y >= GAME_HEIGHT ||
            new_head.x < 0 || new_head.x >= GAME_WIDTH) {
            game_over = true;
            continue;
        }
        // 撞自己
        for (const auto& p : snake) {
            if (p.y == new_head.y && p.x == new_head.x) {
                game_over = true;
                continue;
            }
        }

        // --- 移动身体 ---
        snake.insert(snake.begin(), new_head);

        // 吃食物？
        if (new_head.y == food.y && new_head.x == food.x) {
            score += 10;
            // 生成新食物（确保不在蛇身上）
            bool valid;
            do {
                valid = true;
                food = {rand() % GAME_HEIGHT, rand() % GAME_WIDTH};
                for (const auto& p : snake) {
                    if (p.y == food.y && p.x == food.x) valid = false;
                }
            } while (!valid);
        } else {
            snake.pop_back();  // 没吃到就去掉尾巴
        }

        // ========== 6. 绘制 ==========
        werase(win);
        box(win, 0, 0);  // 重画边框（werase 会清掉它）

        // 画食物
        if (has_colors()) wattron(win, COLOR_PAIR(2));
        mvwaddch(win, food.y + 1, food.x + 1, '*');  // +1 是因为边框占了第0行/列
        if (has_colors()) wattroff(win, COLOR_PAIR(2));

        // 画蛇
        if (has_colors()) wattron(win, COLOR_PAIR(1));
        for (size_t i = 0; i < snake.size(); i++) {
            char c = (i == 0) ? '@' : 'o';
            mvwaddch(win, snake[i].y + 1, snake[i].x + 1, c);
        }
        if (has_colors()) wattroff(win, COLOR_PAIR(1));

        wrefresh(win);

        // 在标准屏幕顶部显示分数
        mvprintw(0, 0, "Score: %d  |  Arrow keys: move  |  Q: quit", score);
        clrtoeol();  // 清除本行剩余内容，避免残影
        refresh();

        usleep(150000);  // 150ms = 约 6.6 FPS，可自行调整
    }

    // ========== 7. 结束画面 ==========
    nodelay(stdscr, FALSE);  // 恢复阻塞输入，等待按键
    mvprintw(term_h/2, (term_w-20)/2, "GAME OVER! Score: %d", score);
    mvprintw(term_h/2 + 1, (term_w-15)/2, "Press any key...");
    refresh();
    getch();

    // ========== 8. 清理 ==========
    delwin(win);
    endwin();
    return 0;
}