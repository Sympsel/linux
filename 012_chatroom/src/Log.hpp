#pragma once

#include <ctime>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace sym {

// ANSI color codes
namespace Color {
    const std::string RESET       = "\033[0m";
    const std::string BLACK       = "\033[30m";
    const std::string RED         = "\033[31m";
    const std::string GREEN       = "\033[32m";
    const std::string YELLOW      = "\033[33m";
    const std::string BLUE        = "\033[34m";
    const std::string MAGENTA     = "\033[35m";
    const std::string CYAN        = "\033[36m";
    const std::string WHITE       = "\033[37m";

    // Bold colors
    const std::string BOLD_BLACK   = "\033[30;1m";
    const std::string BOLD_RED     = "\033[31;1m";
    const std::string BOLD_GREEN   = "\033[32;1m";
    const std::string BOLD_YELLOW  = "\033[33;1m";
    const std::string BOLD_BLUE    = "\033[34;1m";
    const std::string BOLD_MAGENTA = "\033[35;1m";
    const std::string BOLD_CYAN    = "\033[36;1m";
    const std::string BOLD_WHITE   = "\033[37;1m";
}

// 获取当前时间字符串
inline std::string getcurrtime() {
    time_t stamp = time(nullptr);
    struct tm date_time;
    localtime_r(&stamp, &date_time);

    std::stringstream ss;
    ss << std::setw(4) << date_time.tm_year + 1900
       << std::setfill('0') << '-'
       << std::setw(2) << date_time.tm_mon + 1 << '-'
       << std::setw(2) << date_time.tm_mday << ' '
       << std::setw(2) << date_time.tm_hour << ':'
       << std::setw(2) << date_time.tm_min << ':'
       << std::setw(2) << date_time.tm_sec;
    return ss.str();
}

// 获取当前日期字符串（用于日志文件名）
inline std::string getcurrdate() {
    time_t stamp = time(nullptr);
    struct tm date_time;
    localtime_r(&stamp, &date_time);

    std::stringstream ss;
    ss << std::setw(4) << date_time.tm_year + 1900
       << std::setfill('0')
       << std::setw(2) << date_time.tm_mon + 1
       << std::setw(2) << date_time.tm_mday;
    return ss.str();
}

enum class log_level_t {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

inline std::string loglevel2str(const log_level_t& level) {
    switch (level) {
        case log_level_t::DEBUG:   return "DEBUG";
        case log_level_t::INFO:    return "INFO";
        case log_level_t::WARNING: return "WARNING";
        case log_level_t::ERROR:   return "ERROR";
        case log_level_t::FATAL:   return "FATAL";
        default:                   return "UNKNOWN";
    }
}

inline std::string get_color_for_level(const log_level_t& level) {
    switch (level) {
        case log_level_t::DEBUG:   return Color::BOLD_CYAN;
        case log_level_t::INFO:    return Color::BOLD_BLUE;
        case log_level_t::WARNING: return Color::BOLD_YELLOW;
        case log_level_t::ERROR:   return Color::BOLD_RED;
        case log_level_t::FATAL:   return Color::BOLD_MAGENTA;
        default:                   return Color::RESET;
    }
}

// 日志策略接口
class log_strategy_vir {
   public:
    virtual ~log_strategy_vir() = default;
    virtual void synclog(const std::string& logmsg) = 0;
    virtual void set_color_enabled(bool enabled) {}
    virtual bool is_color_enabled() const { return false; }
    virtual void flush() {}
};

// 控制台日志策略
class console_log_strategy : public log_strategy_vir {
   public:
    explicit console_log_strategy(bool color_enabled = true)
        : _color_enabled(color_enabled) {}

    ~console_log_strategy() override = default;

    void synclog(const std::string& logmsg) override {
        std::lock_guard<std::mutex> locker(_lock);
        std::cout << logmsg << std::endl;
    }

    void set_color_enabled(bool enabled) override {
        std::lock_guard<std::mutex> locker(_lock);
        _color_enabled = enabled;
    }

    bool is_color_enabled() const override {
        return _color_enabled;
    }

    void flush() override {
        std::lock_guard<std::mutex> locker(_lock);
        std::cout.flush();
    }

   private:
    mutable std::mutex _lock;
    bool _color_enabled;
};

// 文件日志策略
class file_log_strategy : public log_strategy_vir {
   public:
    explicit file_log_strategy(
        const std::string& logdir = "./log/",
        const std::string& logfilename = "",
        bool daily_file = true)
        : _logdir(logdir),
          _base_filename(logfilename),
          _daily_file(daily_file),
          _current_date(getcurrdate()) {

        // 确保日志目录存在
        std::lock_guard<std::mutex> locker(_lock);
        if (!std::filesystem::exists(_logdir)) {
            try {
                std::filesystem::create_directories(_logdir);
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "[Logger Error] Failed to create log directory: "
                          << e.what() << std::endl;
            }
        }

        // 打开日志文件
        open_log_file();
    }

    ~file_log_strategy() override {
        std::lock_guard<std::mutex> locker(_lock);
        if (_ofs.is_open()) {
            _ofs.close();
        }
    }

    void synclog(const std::string& logmsg) override {
        std::lock_guard<std::mutex> locker(_lock);

        // 检查是否需要切换日志文件（按天）
        if (_daily_file) {
            std::string today = getcurrdate();
            if (today != _current_date) {
                _current_date = today;
                open_log_file();
            }
        }

        // 移除颜色代码并写入文件
        std::string clean_logmsg = remove_color_codes(logmsg);

        if (_ofs.is_open()) {
            _ofs << clean_logmsg << std::endl;
        }
    }

    bool is_color_enabled() const override {
        return false;  // 文件日志不需要颜色
    }

    void flush() override {
        std::lock_guard<std::mutex> locker(_lock);
        if (_ofs.is_open()) {
            _ofs.flush();
        }
    }

    // 获取当前日志文件完整路径
    std::string get_current_logfile() const {
        std::lock_guard<std::mutex> locker(_lock);
        return _logdir + _get_filename();
    }

    static std::string remove_color_codes(const std::string& msg) {
        std::string result;
        result.reserve(msg.size());  // 预分配内存
        bool in_escape = false;
        for (char c : msg) {
            if (c == '\033') {
                in_escape = true;
                continue;
            }
            if (in_escape && c == 'm') {
                in_escape = false;
                continue;
            }
            if (!in_escape) {
                result += c;
            }
        }
        return result;
    }

   private:
    std::string _get_filename() const {
        if (_daily_file) {
            if (_base_filename.empty()) {
                return "log_" + _current_date + ".txt";
            }
            // 在基础文件名前加日期
            size_t dot_pos = _base_filename.find_last_of('.');
            if (dot_pos != std::string::npos) {
                return _base_filename.substr(0, dot_pos) + "_" + _current_date
                       + _base_filename.substr(dot_pos);
            }
            return _base_filename + "_" + _current_date;
        }
        return _base_filename.empty() ? "log.txt" : _base_filename;
    }

    void open_log_file() {
        if (_ofs.is_open()) {
            _ofs.close();
        }
        std::string filepath = _logdir + _get_filename();
        _ofs.open(filepath, std::ios::app);
        if (!_ofs.is_open()) {
            std::cerr << "[Logger Error] Failed to open log file: " << filepath << std::endl;
        }
    }

    std::string _logdir;
    std::string _base_filename;
    bool _daily_file;
    std::string _current_date;
    mutable std::mutex _lock;
    std::ofstream _ofs;
};

// 组合日志策略（同时输出到控制台和文件）
class combined_log_strategy : public log_strategy_vir {
   public:
    combined_log_strategy(
        const std::string& logdir = "./log/",
        const std::string& logfilename = "",
        bool color_enabled = true,
        bool daily_file = true)
        : _console(std::make_unique<console_log_strategy>(color_enabled)),
          _file(std::make_unique<file_log_strategy>(logdir, logfilename, daily_file)) {}

    ~combined_log_strategy() override = default;

    void synclog(const std::string& logmsg) override {
        _console->synclog(logmsg);
        _file->synclog(logmsg);
    }

    void set_color_enabled(bool enabled) override {
        _console->set_color_enabled(enabled);
    }

    bool is_color_enabled() const override {
        return _console->is_color_enabled();
    }

    void flush() override {
        _console->flush();
        _file->flush();
    }

   private:
    std::unique_ptr<console_log_strategy> _console;
    std::unique_ptr<file_log_strategy> _file;
};

// 日志器主类
class logger {
   public:
    logger() {
        use_console_log_strategy();
    }

    ~logger() = default;

    // 禁用拷贝和移动
    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;
    logger(logger&&) = delete;
    logger& operator=(logger&&) = delete;

    void use_console_log_strategy(bool color_enabled = true) {
        std::lock_guard<std::mutex> locker(_strategy_lock);
        _strategy = std::make_unique<console_log_strategy>(color_enabled);
    }

    void use_file_log_strategy(
        const std::string& logdir = "./log/",
        const std::string& logfilename = "",
        bool daily_file = true) {
        std::lock_guard<std::mutex> locker(_strategy_lock);
        _strategy = std::make_unique<file_log_strategy>(logdir, logfilename, daily_file);
    }

    void use_combined_log_strategy(
        const std::string& logdir = "./log/",
        const std::string& logfilename = "",
        bool color_enabled = true,
        bool daily_file = true) {
        std::lock_guard<std::mutex> locker(_strategy_lock);
        _strategy = std::make_unique<combined_log_strategy>(logdir, logfilename, color_enabled, daily_file);
    }

    void set_color_enabled(bool enabled) {
        std::lock_guard<std::mutex> locker(_strategy_lock);
        if (_strategy) {
            _strategy->set_color_enabled(enabled);
        }
    }

    bool is_color_enabled() const {
        std::lock_guard<std::mutex> locker(_strategy_lock);
        return _strategy ? _strategy->is_color_enabled() : false;
    }

    void flush() {
        std::lock_guard<std::mutex> locker(_strategy_lock);
        if (_strategy) {
            _strategy->flush();
        }
    }

    // 日志消息类（RAII 风格）
    class logmsg {
       public:
        logmsg(const log_level_t level,
               const std::string& filename,
               const int line,
               logger& self)
            : _level(level),
              _curr_time(getcurrtime()),
              _pid(getpid()),
              _filename(filename),
              _line(line),
              _self(self) {

            std::stringstream ss;

            // 添加颜色（如果启用）
            if (_self.is_color_enabled()) {
                ss << get_color_for_level(_level);
            }

            ss << '[' << _curr_time << " | "
               << loglevel2str(_level) << " | "
               << "PID:" << _pid << " | "
               << _filename << ':' << _line << "] ";

            _loginfo = ss.str();
        }

        ~logmsg() {
            // 重置颜色
            if (_self.is_color_enabled()) {
                _loginfo += Color::RESET;
            }

            // 输出日志
            _self._synclog(_loginfo);
        }

        // 支持各种类型输入
        template <class T>
        logmsg& operator<<(const T& info) {
            std::stringstream ss;
            ss << info;
            _loginfo += ss.str();
            return *this;
        }

        // 支持流操控符（如 std::endl）
        logmsg& operator<<(std::ostream& (*manip)(std::ostream&)) {
            std::stringstream ss;
            ss << manip;
            _loginfo += ss.str();
            return *this;
        }

       private:
        log_level_t _level;
        std::string _curr_time;
        pid_t _pid;
        std::string _filename;
        int _line;
        std::string _loginfo;
        logger& _self;
    };

    logmsg operator()(const log_level_t level,
                      const std::string& filename,
                      const int line) {
        return logmsg(level, filename, line, *this);
    }

   private:
    void _synclog(const std::string& msg) {
        std::lock_guard<std::mutex> locker(_strategy_lock);
        if (_strategy) {
            _strategy->synclog(msg);
        }
    }

    mutable std::mutex _strategy_lock;
    std::unique_ptr<log_strategy_vir> _strategy;
};

// 全局日志实例
inline logger& get_logger() {
    static logger instance;
    return instance;
}

// 便捷宏定义
#define LOG(level) sym::get_logger()(level, __FILE__, __LINE__)
#define LOG_DEBUG() LOG(sym::log_level_t::DEBUG)
#define LOG_INFO()  LOG(sym::log_level_t::INFO)
#define LOG_WARN()  LOG(sym::log_level_t::WARNING)
#define LOG_ERROR() LOG(sym::log_level_t::ERROR)
#define LOG_FATAL() LOG(sym::log_level_t::FATAL)

#define USE_CONSOLE_LOG(color) sym::get_logger().use_console_log_strategy(color)
#define USE_FILE_LOG(dir, name, daily) sym::get_logger().use_file_log_strategy(dir, name, daily)
#define USE_COMBINED_LOG(dir, name, color, daily) sym::get_logger().use_combined_log_strategy(dir, name, color, daily)
#define SET_LOG_COLOR(enabled) sym::get_logger().set_color_enabled(enabled)
#define FLUSH_LOG() sym::get_logger().flush()

}  // namespace sym