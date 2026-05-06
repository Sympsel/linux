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
#include <utility>

namespace sym {
    // ANSI color codes
    namespace Color {
        const std::string RESET = "\033[0m";
        const std::string BLACK = "\033[30m";
        const std::string RED = "\033[31m";
        const std::string GREEN = "\033[32m";
        const std::string YELLOW = "\033[33m";
        const std::string BLUE = "\033[34m";
        const std::string MAGENTA = "\033[35m";
        const std::string CYAN = "\033[36m";
        const std::string WHITE = "\033[37m";

        // Bold colors
        const std::string BOLD_BLACK = "\033[30;1m";
        const std::string BOLD_RED = "\033[31;1m";
        const std::string BOLD_GREEN = "\033[32;1m";
        const std::string BOLD_YELLOW = "\033[33;1m";
        const std::string BOLD_BLUE = "\033[34;1m";
        const std::string BOLD_MAGENTA = "\033[35;1m";
        const std::string BOLD_CYAN = "\033[36;1m";
        const std::string BOLD_WHITE = "\033[37;1m";
    }

    // 获取当前时间字符串
    inline std::string GetCurrTime() {
        const time_t stamp = time(nullptr);
        tm date_time{};
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
    inline std::string GetCurrDate() {
        const time_t stamp = time(nullptr);
        tm date_time{};
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

    inline std::string loglevel2str(const log_level_t &level) {
        switch (level) {
            case log_level_t::DEBUG: return "DEBUG";
            case log_level_t::INFO: return "INFO";
            case log_level_t::WARNING: return "WARNING";
            case log_level_t::ERROR: return "ERROR";
            case log_level_t::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }

    inline std::string GetColorForLevel(const log_level_t &level) {
        switch (level) {
            case log_level_t::DEBUG: return Color::BOLD_CYAN;
            case log_level_t::INFO: return Color::BOLD_BLUE;
            case log_level_t::WARNING: return Color::BOLD_YELLOW;
            case log_level_t::ERROR: return Color::BOLD_RED;
            case log_level_t::FATAL: return Color::BOLD_MAGENTA;
            default: return Color::RESET;
        }
    }

    // 日志策略接口
    class LogStrategyVirtual {
    public:
        virtual ~LogStrategyVirtual() = default;

        virtual void SyncLog(const std::string &) = 0;

        virtual void SetColorEnabled(bool) {
        }

        [[nodiscard]] virtual bool IsColorEnabled() const { return false; }

        virtual void Flush() {
        }
    };

    // 控制台日志策略
    class ConsoleLogStrategy : public LogStrategyVirtual {
    public:
        explicit ConsoleLogStrategy(const bool color_enabled = true)
            : _color_enabled(color_enabled) {
        }

        ~ConsoleLogStrategy() override = default;

        void SyncLog(const std::string &log_msg) override {
            std::lock_guard locker(_lock);
            std::cout << log_msg << std::endl;
        }

        void SetColorEnabled(const bool enabled) override {
            std::lock_guard locker(_lock);
            _color_enabled = enabled;
        }

        bool IsColorEnabled() const override {
            return _color_enabled;
        }

        void Flush() override {
            std::lock_guard locker(_lock);
            std::cout.flush();
        }

    private:
        mutable std::mutex _lock;
        bool _color_enabled;
    };

    // 文件日志策略
    class file_log_strategy : public LogStrategyVirtual {
    public:
        explicit file_log_strategy(
            std::string logdir = "./log/",
            std::string log_filename = "",
            bool daily_file = true)
            : _logdir(std::move(logdir)),
              _base_filename(std::move(log_filename)),
              _daily_file(daily_file),
              _current_date(GetCurrDate()) {
            // 确保日志目录存在
            std::lock_guard locker(_lock);
            if (!std::filesystem::exists(_logdir)) {
                try {
                    std::filesystem::create_directories(_logdir);
                } catch (const std::filesystem::filesystem_error &e) {
                    std::cerr << "[Logger Error] Failed to create log directory: "
                            << e.what() << std::endl;
                }
            }

            // 打开日志文件
            OpenLogFile();
        }

        ~file_log_strategy() override {
            std::lock_guard locker(_lock);
            if (_ofs.is_open()) {
                _ofs.close();
            }
        }

        void SyncLog(const std::string &log_msg) override {
            std::lock_guard locker(_lock);

            // 检查是否需要切换日志文件（按天）
            if (_daily_file) {
                if (const std::string today = GetCurrDate(); today != _current_date) {
                    _current_date = today;
                    OpenLogFile();
                }
            }

            // 移除颜色代码并写入文件
            const std::string clean_log_msg = remove_color_codes(log_msg);

            if (_ofs.is_open()) {
                _ofs << clean_log_msg << std::endl;
            }
        }

        bool IsColorEnabled() const override {
            return false; // 文件日志不需要颜色
        }

        void Flush() override {
            std::lock_guard locker(_lock);
            if (_ofs.is_open()) {
                _ofs.flush();
            }
        }

        // 获取当前日志文件完整路径
        std::string get_current_logfile() const {
            std::lock_guard locker(_lock);
            return _logdir + _get_filename();
        }

        static std::string remove_color_codes(const std::string &msg) {
            std::string result;
            result.reserve(msg.size()); // 预分配内存
            bool in_escape = false;
            for (const char c: msg) {
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
                if (const size_t dot_pos = _base_filename.find_last_of('.'); dot_pos != std::string::npos) {
                    return _base_filename.substr(0, dot_pos) + "_" + _current_date
                           + _base_filename.substr(dot_pos);
                }
                return _base_filename + "_" + _current_date;
            }
            return _base_filename.empty() ? "log.txt" : _base_filename;
        }

        void OpenLogFile() {
            if (_ofs.is_open()) {
                _ofs.close();
            }
            const std::string filepath = _logdir + _get_filename();
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
    class CombinedLogStrategy : public LogStrategyVirtual {
    public:
        explicit CombinedLogStrategy(
            const std::string &logdir = "./log/",
            const std::string &log_filename = "",
            bool color_enabled = true,
            bool daily_file = true)
            : _console(std::make_unique<ConsoleLogStrategy>(color_enabled)),
              _file(std::make_unique<file_log_strategy>(logdir, log_filename, daily_file)) {
        }

        ~CombinedLogStrategy() override = default;

        void SyncLog(const std::string &log_msg) override {
            _console->SyncLog(log_msg);
            _file->SyncLog(log_msg);
        }

        void SetColorEnabled(const bool enabled) override {
            _console->SetColorEnabled(enabled);
        }

        [[nodiscard]] bool IsColorEnabled() const override {
            return _console->IsColorEnabled();
        }

        void Flush() override {
            _console->Flush();
            _file->Flush();
        }

    private:
        std::unique_ptr<ConsoleLogStrategy> _console;
        std::unique_ptr<file_log_strategy> _file;
    };

    // 日志器主类
    class logger {
    public:
        logger() {
            UseConsoleLog();
        }

        ~logger() = default;

        // 禁用拷贝和移动
        logger(const logger &) = delete;

        logger &operator=(const logger &) = delete;

        logger(logger &&) = delete;

        logger &operator=(logger &&) = delete;

        void UseConsoleLog(bool color_enabled = true) {
            std::lock_guard locker(_strategy_lock);
            _strategy = std::make_unique<ConsoleLogStrategy>(color_enabled);
        }

        void UseFileLogStrategy(
            const std::string &logdir = "./log/",
            const std::string &log_filename = "",
            bool daily_file = true) {
            std::lock_guard locker(_strategy_lock);
            _strategy = std::make_unique<file_log_strategy>(logdir, log_filename, daily_file);
        }

        void UseCombinedLogStrategy(
            const std::string &log_dir = "./log/",
            const std::string &log_filename = "",
            bool color_enabled = true,
            bool daily_file = true) {
            std::lock_guard locker(_strategy_lock);
            _strategy = std::make_unique<CombinedLogStrategy>(log_dir, log_filename, color_enabled, daily_file);
        }

        void SetColorEnabled(const bool enabled) const {
            std::lock_guard locker(_strategy_lock);
            if (_strategy) {
                _strategy->SetColorEnabled(enabled);
            }
        }

        bool IsColorEnabled() const {
            std::lock_guard locker(_strategy_lock);
            return _strategy ? _strategy->IsColorEnabled() : false;
        }

        void Flush() const {
            std::lock_guard locker(_strategy_lock);
            if (_strategy) {
                _strategy->Flush();
            }
        }

        // 日志消息类（RAII 风格）
        class LogMsg {
        public:
            LogMsg(const log_level_t level,
                   std::string filename,
                   const int line,
                   logger &self)
                : _level(level),
                  _curr_time(GetCurrTime()),
                  _pid(getpid()),
                  _filename(std::move(filename)),
                  _line(line),
                  _self(self) {
                std::stringstream ss;

                // 添加颜色（如果启用）
                if (_self.IsColorEnabled()) {
                    ss << GetColorForLevel(_level);
                }

                ss << '[' << _curr_time << " | "
                        << loglevel2str(_level) << " | "
                        << "PID:" << _pid << " | "
                        << _filename << ':' << _line << "] ";

                _log_info = ss.str();
            }

            ~LogMsg() {
                // 重置颜色
                if (_self.IsColorEnabled()) {
                    _log_info += Color::RESET;
                }

                // 输出日志
                _self.SyncLog(_log_info);
            }

            // 支持各种类型输入
            template<class T>
            LogMsg &operator<<(const T &info) {
                std::stringstream ss;
                ss << info;
                _log_info += ss.str();
                return *this;
            }

            // 支持流操控符（如 std::endl）
            LogMsg &operator<<(std::ostream & (*manip)(std::ostream &)) {
                std::stringstream ss;
                ss << manip;
                _log_info += ss.str();
                return *this;
            }

        private:
            log_level_t _level;
            std::string _curr_time;
            pid_t _pid;
            std::string _filename;
            int _line;
            std::string _log_info;
            logger &_self;
        };

        LogMsg operator()(const log_level_t level,
                          const std::string &filename,
                          const int line) {
            return {level, filename, line, *this};
        }

    private:
        void SyncLog(const std::string &msg) const {
            std::lock_guard locker(_strategy_lock);
            if (_strategy) {
                _strategy->SyncLog(msg);
            }
        }

        mutable std::mutex _strategy_lock;
        std::unique_ptr<LogStrategyVirtual> _strategy;
    };

    // 全局日志实例
    inline logger &get_logger() {
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

#define USE_CONSOLE_LOG(color) sym::get_logger().UseConsoleLog(color)
#define USE_FILE_LOG(dir, name, daily) sym::get_logger().UseFileLogStrategy(dir, name, daily)
#define USE_COMBINED_LOG(dir, name, color, daily) sym::get_logger().UseCombinedLogStrategy(dir, name, color, daily)
#define SET_LOG_COLOR(enabled) sym::get_logger().SetColorEnabled(enabled)
#define FLUSH_LOG() sym::get_logger().Flush()
} // namespace sym
