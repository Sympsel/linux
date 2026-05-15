// #include <memory>
//
// #include "Conf.hpp"
// #include "Game.hpp"
// #include "Log.hpp"
//
// using namespace sym;
// Sym::Conf conf;
//
// int main(const int argc, char *argv[]) {
//     // 使用绝对路径，确保日志文件创建在正确位置
//     const auto log_dir = std::filesystem::canonical(std::filesystem::current_path());
//
//     // 使用组合日志策略（同时输出到控制台和文件）
//     USE_COMBINED_LOG(log_dir.string(), "snake.log", true, true);
//
//     LOG_INFO() << "Config loading!";
//     conf.Load();
//     LOG_INFO() << "Config loaded!";
//
//     while (true) {
//         const auto game = std::make_unique<Sym::Game>(
//             conf["width"], conf["height"], conf["def_len"]
//         );
//         if (const bool is_replay = game->Run(); !is_replay) {
//             break;
//         }
//     }
//     FLUSH_LOG();
//     return 0;
// }

#include <memory>
#include <iostream>

#include "Conf.hpp"
#include "Game.hpp"
#include "Log.hpp"

using namespace sym;
Sym::Conf conf;

int main(const int argc, char *argv[]) {
    // 获取当前工作目录
    auto current_path = std::filesystem::current_path();
    std::cerr << "[DEBUG] Current path: " << current_path.string() << std::endl;

    // 使用绝对路径
    std::string log_dir = current_path.string();
    if (!log_dir.ends_with('/')) {
        log_dir += '/';
    }

    std::cerr << "[DEBUG] Log directory: " << log_dir << std::endl;
    std::cerr << "[DEBUG] Setting up combined log strategy..." << std::endl;

    USE_FILE_LOG(log_dir, "snake.log", false);

    std::cerr << "[DEBUG] Log strategy set, testing log..." << std::endl;

    LOG_INFO() << "Config loading!";
    std::cerr << "[DEBUG] First log written" << std::endl;

    try {
        conf.Load();
    } catch (const std::exception &e) {
        LOG_ERROR() << "Failed to load config: " << e.what();
        std::cerr << "[ERROR] Config load failed: " << e.what() << std::endl;
        return 1;
    }

    LOG_INFO() << "Config loaded!";
    FLUSH_LOG();
    std::cerr << "[DEBUG] Config loaded, logs flushed" << std::endl;

    while (true) {
        const auto game = std::make_unique<Sym::Game>(
            conf["width"], conf["height"], conf["def_len"]
        );
        if (const bool is_replay = game->Run(); !is_replay) {
            break;
        }
    }

    // 确保所有日志都写入文件
    FLUSH_LOG();
    std::cerr << "[DEBUG] Program exiting normally" << std::endl;

    return 0;
}
