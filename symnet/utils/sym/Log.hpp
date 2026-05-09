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

    /**
     * @brief Gets the current time as a formatted string.
     * @return Time string in format "YYYY-MM-DD HH:MM:SS"
     */
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

    /**
     * @brief Gets the current date as a compact string (for log filenames).
     * @return Date string in format "YYYYMMDD"
     */
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

    /**
     * @brief Enumeration of log severity levels.
     */
    enum class log_level_t {
        DEBUG, ///< Debug-level messages for development
        INFO, ///< Informational messages about normal operation
        WARNING, ///< Warning messages about potential issues
        ERROR, ///< Error messages about failures
        FATAL ///< Fatal error messages before termination
    };

    /**
     * @brief Converts a log level enum to its string representation.
     * @param level Log level to convert
     * @return String representation of the log level
     */
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

    /**
     * @brief Gets the ANSI color code for a given log level.
     * @param level Log level
     * @return ANSI color code string
     */
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

    /**
     * @brief Interface for log output strategies.
     *
     * Defines the contract for different logging destinations
     * (console, file, combined, etc.).
     */
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

    /**
     * @brief Console logging strategy with optional color support.
     *
     * Outputs log messages to stdout with thread-safe access.
     */
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

    /**
     * @brief File-based logging strategy with daily rotation support.
     *
     * Writes log messages to files with automatic directory creation
     * and optional daily file rotation. Strips ANSI color codes for file output.
     */
    class FileLogStrategy : public LogStrategyVirtual {
    public:
        explicit FileLogStrategy(
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
            OpenLogFile();
        }

        ~FileLogStrategy() override {
            std::lock_guard locker(_lock);
            if (_ofs.is_open()) {
                _ofs.close();
            }
        }

        /**
         * @brief Synchronously writes a log message to the file.
         * @param log_msg Formatted log message (color codes will be stripped)
         */
        void SyncLog(const std::string &log_msg) override {
            std::lock_guard locker(_lock);

            if (_daily_file) {
                if (const std::string today = GetCurrDate(); today != _current_date) {
                    _current_date = today;
                    OpenLogFile();
                }
            }

            const std::string clean_log_msg = RemoveColorCodes(log_msg);

            if (_ofs.is_open()) {
                _ofs << clean_log_msg << std::endl;
            }
        }

        bool IsColorEnabled() const override {
            return false;
        }

        /**
         * @brief Flushes the file buffer to disk.
         */
        void Flush() override {
            std::lock_guard locker(_lock);
            if (_ofs.is_open()) {
                _ofs.flush();
            }
        }

        /**
         * @brief Gets the full path of the current log file.
         * @return Complete file path including directory and filename
         */
        std::string GetCurrentLogfile() const {
            std::lock_guard locker(_lock);
            return _logdir + GetFilenameHelper();
        }

        /**
         * @brief Removes ANSI color codes from a log message.
         * @param msg Message potentially containing color codes
         * @return Clean message without color codes
         */
        static std::string RemoveColorCodes(const std::string &msg) {
            std::string result;
            result.reserve(msg.size());
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
        std::string GetFilenameHelper() const {
            if (_daily_file) {
                if (_base_filename.empty()) {
                    return "log_" + _current_date + ".txt";
                }

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
            const std::string filepath = _logdir + GetFilenameHelper();
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

    /**
     * @brief Combined logging strategy that outputs to both console and file.
     *
     * Delegates log messages to both ConsoleLogStrategy and file_log_strategy
     * instances, providing dual output capability.
     */
    class CombinedLogStrategy : public LogStrategyVirtual {
    public:
        explicit CombinedLogStrategy(
            const std::string &logdir = "./log/",
            const std::string &log_filename = "",
            bool color_enabled = true,
            bool daily_file = true)
            : _console(std::make_unique<ConsoleLogStrategy>(color_enabled)),
              _file(std::make_unique<FileLogStrategy>(logdir, log_filename, daily_file)) {
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
        std::unique_ptr<FileLogStrategy> _file;
    };

    /**
     * @brief Main logger class providing flexible logging capabilities.
     *
     * Singleton class that manages log strategies and provides a unified
     * interface for logging. Supports runtime strategy switching between
     * console, file, and combined logging modes.
     */
    class logger {
    public:
        logger() {
            UseConsoleLog();
        }

        ~logger() = default;

        logger(const logger &) = delete;

        logger &operator=(const logger &) = delete;

        logger(logger &&) = delete;

        logger &operator=(logger &&) = delete;

        /**
         * @brief Configures the logger to use console output.
         * @param color_enabled Enable/disable colored output (default: true)
         */
        void UseConsoleLog(bool color_enabled = true) {
            std::lock_guard locker(_strategy_lock);
            _strategy = std::make_unique<ConsoleLogStrategy>(color_enabled);
        }

        /**
         * @brief Configures the logger to use file output.
         * @param logdir Directory for log files (default: "./log/")
         * @param log_filename Custom filename (default: auto-generated with date)
         * @param daily_file Enable daily file rotation (default: true)
         */
        void UseFileLogStrategy(
            const std::string &logdir = "./log/",
            const std::string &log_filename = "",
            bool daily_file = true) {
            std::lock_guard locker(_strategy_lock);
            _strategy = std::make_unique<FileLogStrategy>(logdir, log_filename, daily_file);
        }

        /**
         * @brief Configures the logger to use combined console and file output.
         * @param log_dir Directory for log files
         * @param log_filename Custom filename
         * @param color_enabled Enable/disable colored console output
         * @param daily_file Enable daily file rotation
         */
        void UseCombinedLogStrategy(
            const std::string &log_dir = "./log/",
            const std::string &log_filename = "",
            bool color_enabled = true,
            bool daily_file = true) {
            std::lock_guard locker(_strategy_lock);
            _strategy = std::make_unique<CombinedLogStrategy>(log_dir, log_filename, color_enabled, daily_file);
        }

        /**
         * @brief Enables or disables colored console output.
         * @param enabled true to enable colors, false to disable
         */
        void SetColorEnabled(const bool enabled) const {
            std::lock_guard locker(_strategy_lock);
            if (_strategy) {
                _strategy->SetColorEnabled(enabled);
            }
        }

        /**
         * @brief Checks if colored output is enabled.
         * @return true if colors are enabled, false otherwise
         */
        bool IsColorEnabled() const {
            std::lock_guard locker(_strategy_lock);
            return _strategy ? _strategy->IsColorEnabled() : false;
        }

        /**
         * @brief Flushes all log output buffers.
         */
        void Flush() const {
            std::lock_guard locker(_strategy_lock);
            if (_strategy) {
                _strategy->Flush();
            }
        }

        /**
         * @brief RAII class for building and outputting log messages.
         *
         * Captures log context (timestamp, PID, file, line) on construction
         * and outputs the complete message on destruction.
         */
        class LogMsg {
        public:
            /**
             * @brief Constructs a log message with context information.
             * @param level Severity level
             * @param filename Source file name
             * @param line Line number in source file
             * @param self Reference to the logger instance
             */
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

                if (_self.IsColorEnabled()) {
                    ss << GetColorForLevel(_level);
                }

                ss << '[' << _curr_time << " | "
                        << loglevel2str(_level) << " | "
                        << "PID:" << _pid << " | "
                        << _filename << ':' << _line << "] ";

                _log_info = ss.str();
            }

            /**
             * @brief Destructor that finalizes and outputs the log message.
             */
            ~LogMsg() {
                if (_self.IsColorEnabled()) {
                    _log_info += Color::RESET;
                }

                _self.SyncLog(_log_info);
            }

            /**
             * @brief Appends data to the log message.
             * @param info Any streamable data to append
             * @return Reference to self for chaining
             */
            template<class T>
            LogMsg &operator<<(const T &info) {
                std::stringstream ss;
                ss << info;
                _log_info += ss.str();
                return *this;
            }

            /**
             * @brief Appends stream manipulators (e.g., std::endl).
             * @param manip Stream manipulator function
             * @return Reference to self for chaining
             */
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

        /**
         * @brief Creates a new log message with the specified level and location.
         * @param level Severity level
         * @param filename Source file name (typically __FILE__)
         * @param line Line number (typically __LINE__)
         * @return LogMsg object for streaming log content
         */
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

    /**
     * @brief Gets the global logger singleton instance.
     * @return Reference to the singleton logger
     */
    inline logger &get_logger() {
        static logger instance;
        return instance;
    }

    // Convenience macros for logging
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
