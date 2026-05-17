#include <memory>
#include <iostream>

#include "Conf.hpp"
#include "Game.hpp"
#include "Log.hpp"

using namespace sym;
Sym::Conf conf;

int main(const int argc, char *argv[]) {
    // 获取当前工作目录
    const auto current_path = std::filesystem::current_path();
    std::cerr << "[DEBUG] Current path: " << current_path.string() << std::endl;

    // 使用绝对路径
    std::string log_dir = current_path.string();
    if (log_dir.back() != '/') {
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

    // 关闭音频线程池
    LOG_INFO() << "Shutting down audio thread pool...";
    auto& audio_pool = ThreadPool<std::function<void()>>::GetInstance();
    audio_pool.Quit();
    audio_pool.Stop();
    audio_pool.Wait();
    LOG_INFO() << "Audio thread pool stopped";

    std::cerr << "[DEBUG] Program exiting normally" << std::endl;
    return 0;
}
