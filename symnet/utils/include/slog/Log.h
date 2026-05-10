#pragma once

#include <fstream>
#include <iomanip>
#include <string>
#include <memory>
#include <mutex>

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
    std::string GetCurrTime();

    /**
     * @brief Gets the current date as a compact string (for log filenames).
     * @return Date string in format "YYYYMMDD"
     */
    std::string GetCurrDate();

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
    std::string LogLevelToStr(const log_level_t &level);

    /**
     * @brief Gets the ANSI color code for a given log level.
     * @param level Log level
     * @return ANSI color code string
     */
    std::string GetColorForLevel(const log_level_t &level);

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

        virtual void SetColorEnabled(bool);

        [[nodiscard]] virtual bool IsColorEnabled() const;

        virtual void Flush();
    };

    /**
     * @brief Console logging strategy with optional color support.
     *
     * Outputs log messages to stdout with thread-safe access.
     */
    class ConsoleLogStrategy : public LogStrategyVirtual {
    public:
        explicit ConsoleLogStrategy(bool color_enabled = true);

        ~ConsoleLogStrategy() override = default;

        void SyncLog(const std::string &log_msg) override;

        void SetColorEnabled(bool enabled) override;

        [[nodiscard]] bool IsColorEnabled() const override;

        void Flush() override;

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
            bool daily_file = true);

        ~FileLogStrategy() override;

        void SyncLog(const std::string &log_msg) override;

        [[nodiscard]] bool IsColorEnabled() const override;

        void Flush() override;

        std::string GetCurrentLogfile() const;

        static std::string RemoveColorCodes(const std::string &msg);

    private:
        std::string GetFilenameHelper() const;

        void OpenLogFile();

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
            bool daily_file = true);

        ~CombinedLogStrategy() override = default;

        void SyncLog(const std::string &log_msg) override;

        void SetColorEnabled(bool enabled) override;

        [[nodiscard]] bool IsColorEnabled() const override;

        void Flush() override;

    private:
        std::unique_ptr<ConsoleLogStrategy> _console;
        std::unique_ptr<FileLogStrategy> _file;
    };

    /**
     * @brief Main Logger class providing flexible logging capabilities.
     *
     * Singleton class that manages log strategies and provides a unified
     * interface for logging. Supports runtime strategy switching between
     * console, file, and combined logging modes.
     */
    class Logger {
    public:
        Logger();

        ~Logger() = default;

        Logger(const Logger &) = delete;

        Logger &operator=(const Logger &) = delete;

        Logger(Logger &&) = delete;

        Logger &operator=(Logger &&) = delete;

        void UseConsoleLog(bool color_enabled = true);

        void UseFileLogStrategy(
            const std::string &logdir = "./log/",
            const std::string &log_filename = "",
            bool daily_file = true);

        void UseCombinedLogStrategy(
            const std::string &log_dir = "./log/",
            const std::string &log_filename = "",
            bool color_enabled = true,
            bool daily_file = true);

        void SetColorEnabled(bool enabled) const;

        [[nodiscard]] bool IsColorEnabled() const;

        void Flush() const;

        /**
         * @brief RAII class for building and outputting log messages.
         *
         * Captures log context (timestamp, PID, file, line) on construction
         * and outputs the complete message on destruction.
         */
        class LogMsg {
        public:
            LogMsg(log_level_t level,
                   std::string filename,
                   int line,
                   Logger &self);

            ~LogMsg();

            LogMsg &operator<<(const auto &info) {
                std::stringstream ss;
                ss << info;
                _log_info += ss.str();
                return *this;
            }

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
            Logger &_self;
        };

        LogMsg operator()(log_level_t level,
                          const std::string &filename,
                          int line);

    private:
        void SyncLog(const std::string &msg) const;

        mutable std::mutex _strategy_lock;
        std::unique_ptr<LogStrategyVirtual> _strategy;
    };

    /**
     * @brief Gets the global Logger singleton instance.
     * @return Reference to the singleton Logger
     */
    Logger &GetLogger();

    // Convenience macros for logging
#define LOG(level) sym::GetLogger()(level, __FILE__, __LINE__)
#define LOG_DEBUG() LOG(sym::log_level_t::DEBUG)
#define LOG_INFO()  LOG(sym::log_level_t::INFO)
#define LOG_WARN()  LOG(sym::log_level_t::WARNING)
#define LOG_ERROR() LOG(sym::log_level_t::ERROR)
#define LOG_FATAL() LOG(sym::log_level_t::FATAL)

#define USE_CONSOLE_LOG(color) sym::GetLogger().UseConsoleLog(color)
#define USE_FILE_LOG(dir, name, daily) sym::GetLogger().UseFileLogStrategy(dir, name, daily)
#define USE_COMBINED_LOG(dir, name, color, daily) sym::GetLogger().UseCombinedLogStrategy(dir, name, color, daily)
#define SET_LOG_COLOR(enabled) sym::GetLogger().SetColorEnabled(enabled)
#define FLUSH_LOG() sym::GetLogger().Flush()
} // namespace sym
