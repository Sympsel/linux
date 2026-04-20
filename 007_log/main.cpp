#include "log.hpp"

int main() {
    // 使用控制台输出（带颜色）
    sym::LOG(sym::log_level_t::DEBUG) << "This is debug message";
    sym::LOG(sym::log_level_t::INFO) << "This is info message";
    sym::LOG(sym::log_level_t::WARNING) << "This is warning message";
    sym::LOG(sym::log_level_t::ERROR) << "This is error message";
    sym::LOG(sym::log_level_t::FATAL) << "This is fatal message";
    
    // 可以关闭颜色
    sym::log.set_color_enabled(false);
    sym::LOG(sym::log_level_t::INFO) << "This is without color";
    
    // 切换到文件输出（自动不带颜色）
    sym::log.use_file_log_strategy();
    sym::LOG(sym::log_level_t::INFO) << "This goes to file without colors";
    
    return 0;
}