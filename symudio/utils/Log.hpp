#pragma once

#include <ctime>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <print>
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
    inline std::string getCurrTime() {
        const time_t stamp = time(nullptr);
        tm dateTime{};
        localtime_r(&stamp, &dateTime);
        return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
                           dateTime.tm_year + 1900,
                           dateTime.tm_mon + 1,
                           dateTime.tm_mday,
                           dateTime.tm_hour,
                           dateTime.tm_min,
                           dateTime.tm_sec
        );
    }

    /**
     * @brief Gets the current date as a compact string (for log filenames).
     * @return Date string in format "YYYYMMDD"
     */
    inline std::string getCurrDate() {
        const time_t stamp = time(nullptr);
        tm dateTime{};
        localtime_r(&stamp, &dateTime);

        return std::format("{:04}{:02}{:02}",
                           dateTime.tm_year + 1900,
                           dateTime.tm_mon + 1,
                           dateTime.tm_mday
        );
    }

    /**
     * @brief Enumeration of log severity levels.
     */
    enum class LogLevel {
        DEBUG, ///< Debug-level messages for development
        INFO, ///< Informational messages about normal operation
        WARNING, ///< Warning messages about potential issues
        ERROR, ///< Error messages about failures
        FATAL ///< Fatal error messages before termination
    };

    /**
     *
     * @brief Converts a log level enum to its string representation.
     * @param level Log level to convert
     * @return String representation of the log level
     */
    inline std::string logLevelToStr(const LogLevel &level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }

    /**
     * @brief Gets the ANSI color code for a given log level.
     * @param level Log level
     * @return ANSI color code string
     */
    inline std::string getColorForLevel(const LogLevel &level) {
        switch (level) {
            case LogLevel::DEBUG: return Color::BOLD_CYAN;
            case LogLevel::INFO: return Color::BOLD_BLUE;
            case LogLevel::WARNING: return Color::BOLD_YELLOW;
            case LogLevel::ERROR: return Color::BOLD_RED;
            case LogLevel::FATAL: return Color::BOLD_MAGENTA;
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

        virtual void syncLog(const std::string &) = 0;

        virtual void setColorEnabled(bool) {
        }

        [[nodiscard]] virtual bool isColorEnabled() const { return false; }

        virtual void flush() {
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
            : _colorEnabled(color_enabled) {
        }

        ~ConsoleLogStrategy() override = default;

        void syncLog(const std::string &logMsg) override {
            std::lock_guard locker(_lock);
            std::println("{}", logMsg);
        }

        void setColorEnabled(const bool enabled) override {
            std::lock_guard locker(_lock);
            _colorEnabled = enabled;
        }

        bool isColorEnabled() const override {
            return _colorEnabled;
        }

    private:
        mutable std::mutex _lock;
        bool _colorEnabled;
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
            : _logDir(std::move(logdir)),
              _baseFilename(std::move(log_filename)),
              _dailyFile(daily_file),
              _currDate(getCurrDate()) {
            // 确保日志目录存在
            std::lock_guard locker(_lock);
            if (!std::filesystem::exists(_logDir)) {
                try {
                    std::filesystem::create_directories(_logDir);
                } catch (const std::filesystem::filesystem_error &e) {
                    std::println(stderr, "[Logger Error] Failed to create log directory: {}", e.what());
                }
            }
            openLogFile();
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
        void syncLog(const std::string &log_msg) override {
            std::lock_guard locker(_lock);

            if (_dailyFile) {
                if (const std::string today = getCurrDate(); today != _currDate) {
                    _currDate = today;
                    openLogFile();
                }
            }

            const std::string clean_log_msg = removeColorCodes(log_msg);

            if (_ofs.is_open()) {
                _ofs << clean_log_msg << std::endl;
            }
        }

        bool isColorEnabled() const override {
            return false;
        }

        /**
         * @brief Flushes the file buffer to disk.
         */
        void flush() override {
            std::lock_guard locker(_lock);
            if (_ofs.is_open()) {
                _ofs.flush();
            }
        }

        /**
         * @brief Gets the full path of the current log file.
         * @return Complete file path including directory and filename
         */
        std::string getCurrentLogfile() const {
            std::lock_guard locker(_lock);
            return _logDir + getFilenameHelper();
        }

        /**
         * @brief Removes ANSI color codes from a log message.
         * @param msg Message potentially containing color codes
         * @return Clean message without color codes
         */
        static std::string removeColorCodes(const std::string &msg) {
            std::string result;
            result.reserve(msg.size());
            bool inEscape = false;
            for (const char c: msg) {
                if (c == '\033') {
                    inEscape = true;
                    continue;
                }
                if (inEscape && c == 'm') {
                    inEscape = false;
                    continue;
                }
                if (!inEscape) {
                    result += c;
                }
            }
            return result;
        }

    private:
        std::string getFilenameHelper() const {
            if (_dailyFile) {
                if (_baseFilename.empty()) {
                    return std::format("log_{}.{}", _currDate, ".txt");
                }

                if (const size_t dot_pos = _baseFilename.find_last_of('.'); dot_pos != std::string::npos) {
                    return std::format("{}_{}", _baseFilename.substr(0, dot_pos), _currDate);
                }
                return std::format("{}_{}", _baseFilename, _currDate);
            }
            return _baseFilename.empty() ? "log.txt" : _baseFilename;
        }

        void openLogFile() {
            if (_ofs.is_open()) {
                _ofs.close();
            }
            const std::string filepath = _logDir + getFilenameHelper();
            _ofs.open(filepath, std::ios::app);
            if (!_ofs.is_open()) {
                std::println(stderr, "[Logger Error] Failed to open log file: {}", filepath);
            }
        }

        std::string _logDir;
        std::string _baseFilename;
        bool _dailyFile;
        std::string _currDate;
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

        void syncLog(const std::string &log_msg) override {
            _console->syncLog(log_msg);
            _file->syncLog(log_msg);
        }

        void setColorEnabled(const bool enabled) override {
            _console->setColorEnabled(enabled);
        }

        [[nodiscard]] bool isColorEnabled() const override {
            return _console->isColorEnabled();
        }

        void flush() override {
            _console->flush();
            _file->flush();
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
            useConsoleLog();
        }

        ~logger() = default;

        logger(const logger &) = delete;

        logger &operator=(const logger &) = delete;

        logger(logger &&) = delete;

        logger &operator=(logger &&) = delete;

        /**
         * @brief Configures the logger to use console output.
         * @param colorEnabled Enable/disable colored output (default: true)
         */
        void useConsoleLog(bool colorEnabled = true) {
            std::lock_guard locker(_strategyLock);
            _strategy = std::make_unique<ConsoleLogStrategy>(colorEnabled);
        }

        /**
         * @brief Configures the logger to use file output.
         * @param logDir Directory for log files (default: "./log/")
         * @param logFilename Custom filename (default: auto-generated with date)
         * @param dailyFile Enable daily file rotation (default: true)
         */
        void useFileLogStrategy(
            const std::string &logDir = "./log/",
            const std::string &logFilename = "",
            bool dailyFile = true) {
            std::lock_guard locker(_strategyLock);
            _strategy = std::make_unique<FileLogStrategy>(logDir, logFilename, dailyFile);
        }

        /**
         * @brief Configures the logger to use combined console and file output.
         * @param logDir Directory for log files
         * @param logFilename Custom filename
         * @param colorEnabled Enable/disable colored console output
         * @param dailyFile Enable daily file rotation
         */
        void useCombinedLogStrategy(
            const std::string &logDir = "./log/",
            const std::string &logFilename = "",
            bool colorEnabled = true,
            bool dailyFile = true) {
            std::lock_guard locker(_strategyLock);
            _strategy = std::make_unique<CombinedLogStrategy>(logDir, logFilename, colorEnabled, dailyFile);
        }

        /**
         * @brief Enables or disables colored console output.
         * @param enabled true to enable colors, false to disable
         */
        void setColorEnabled(const bool enabled) const {
            std::lock_guard locker(_strategyLock);
            if (_strategy) {
                _strategy->setColorEnabled(enabled);
            }
        }

        /**
         * @brief Checks if colored output is enabled.
         * @return true if colors are enabled, false otherwise
         */
        bool isColorEnabled() const {
            std::lock_guard locker(_strategyLock);
            return _strategy ? _strategy->isColorEnabled() : false;
        }

        /**
         * @brief Flushes all log output buffers.
         */
        void flush() const {
            std::lock_guard locker(_strategyLock);
            if (_strategy) {
                _strategy->flush();
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
            LogMsg(const LogLevel level,
                   std::string filename,
                   const int line,
                   logger &self)
                : _level(level),
                  _currTime(getCurrTime()),
                  _pid(getpid()),
                  _filename(std::move(filename)),
                  _line(line),
                  _self(self) {
                std::stringstream ss;

                if (_self.isColorEnabled()) {
                    ss << getColorForLevel(_level);
                }

                ss << std::format("[{} | {} | PID:{} | {}:{}] ",
                                  _currTime, logLevelToStr(_level), _pid, _filename, _line
                );
                _logInfo = ss.str();
            }

            /**
             * @brief Destructor that finalizes and outputs the log message.
             */
            ~LogMsg() {
                if (_self.isColorEnabled()) {
                    _logInfo += Color::RESET;
                }

                _self.syncLog(_logInfo);
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
                _logInfo += ss.str();
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
                _logInfo += ss.str();
                return *this;
            }

        private:
            LogLevel _level;
            std::string _currTime;
            pid_t _pid;
            std::string _filename;
            int _line;
            std::string _logInfo;
            logger &_self;
        };

        /**
         * @brief Creates a new log message with the specified level and location.
         * @param level Severity level
         * @param filename Source file name (typically __FILE__)
         * @param line Line number (typically __LINE__)
         * @return LogMsg object for streaming log content
         */
        LogMsg operator()(const LogLevel level,
                          const std::string &filename,
                          const int line) {
            return {level, filename, line, *this};
        }

    private:
        void syncLog(const std::string &msg) const {
            std::lock_guard locker(_strategyLock);
            if (_strategy) {
                _strategy->syncLog(msg);
            }
        }

        mutable std::mutex _strategyLock;
        std::unique_ptr<LogStrategyVirtual> _strategy;
    };

    /**
     * @brief Gets the global logger singleton instance.
     * @return Reference to the singleton logger
     */
    inline logger &getLogger() {
        static logger instance;
        return instance;
    }

    // Convenience macros for logging
#define LOG(level) sym::getLogger()(level, __FILE__, __LINE__)
#define LOG_DEBUG() LOG(sym::LogLevel::DEBUG)
#define LOG_INFO()  LOG(sym::LogLevel::INFO)
#define LOG_WARN()  LOG(sym::LogLevel::WARNING)
#define LOG_ERROR() LOG(sym::LogLevel::ERROR)
#define LOG_FATAL() LOG(sym::LogLevel::FATAL)

#define USE_CONSOLE_LOG(color) sym::getLogger().useConsoleLog(color)
#define USE_FILE_LOG(dir, name, daily) sym::getLogger().useFileLogStrategy(dir, name, daily)
#define USE_COMBINED_LOG(dir, name, color, daily) sym::getLogger().useCombinedLogStrategy(dir, name, color, daily)
#define SET_LOG_COLOR(enabled) sym::getLogger().setColorEnabled(enabled)
#define FLUSH_LOG() sym::getLogger().flush()
} // namespace sym
