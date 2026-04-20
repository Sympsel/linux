#ifndef __LOG_HPP
#define __LOG_HPP

#include <time.h>
#include <unistd.h>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace sym {

// ANSI color codes
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BLACK   = "\033[30m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";
    
    // Bold colors
    const std::string BOLD_RED     = "\033[31;1m";
    const std::string BOLD_YELLOW  = "\033[33;1m";
    const std::string BOLD_BLUE    = "\033[34;1m";
    const std::string BOLD_CYAN    = "\033[36;1m";
    const std::string BOLD_MAGENTA = "\033[35;1m";
}

std::string getcurrtime() {
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

enum class log_level_t {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

std::string loglevel2str(const log_level_t& level) {
    switch (level) {
        case log_level_t::DEBUG:
            return "DEBUG";
        case log_level_t::INFO:
            return "INFO";
        case log_level_t::WARNING:
            return "WARNING";
        case log_level_t::ERROR:
            return "ERROR";
        case log_level_t::FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

// Get color for log level
std::string get_color_for_level(const log_level_t& level) {
    switch (level) {
        case log_level_t::DEBUG:
            return Color::BOLD_CYAN;
        case log_level_t::INFO:
            return Color::BOLD_BLUE;
        case log_level_t::WARNING:
            return Color::BOLD_YELLOW;
        case log_level_t::ERROR:
            return Color::BOLD_RED;
        case log_level_t::FATAL:
            return Color::BOLD_MAGENTA;
        default:
            return Color::RESET;
    }
}

class log_strategy_vir {
   public:
    virtual ~log_strategy_vir() = default;
    virtual void synclog(const std::string& logmsg) = 0;

    virtual void set_color_enabled([[maybe_unused]] bool enabled) {}
    virtual bool is_color_enabled() const { return false; }
};

class console_log_strategy : public log_strategy_vir {
   public:
    console_log_strategy(bool color_enabled = true) : _color_enabled(color_enabled) {}
    ~console_log_strategy() {}

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

   private:
    std::mutex _lock;
    bool _color_enabled;
};

static std::string glogdir = "./log/";
static std::string glogfilename = "log.txt";

class file_log_strategy : public log_strategy_vir {
   public:
    file_log_strategy(
        const std::string& logdir = glogdir,
        const std::string& logfilename = glogfilename)
        : _logdir(logdir), _logfilename(logfilename) {
        std::lock_guard<std::mutex> locker(_lock);
        if (std::filesystem::exists(_logdir)) return;
        try {
            std::filesystem::create_directories(_logdir);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << '\n';
        }
    }
    ~file_log_strategy() {}

    void synclog(const std::string& logmsg) override {
        std::lock_guard<std::mutex> locker(_lock);
        // Remove color codes for file output
        std::string clean_logmsg = remove_color_codes(logmsg);
        std::cout << clean_logmsg << std::endl;
    }
    
    bool is_color_enabled() const override {
        return false;
    }
    
   private:
    std::string remove_color_codes(const std::string& msg) {
        std::string result;
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
    
    std::string _logdir;
    std::string _logfilename;
    std::mutex _lock;
};

class logger {
   public:
    void use_console_log_strategy() {
        _strategy = std::make_unique<console_log_strategy>();
    }

    void use_file_log_strategy() {
        _strategy = std::make_unique<file_log_strategy>();
    }
    
    void set_color_enabled(bool enabled) {
        if (_strategy) {
            _strategy->set_color_enabled(enabled);
        }
    }
    
    bool is_color_enabled() const {
        return _strategy ? _strategy->is_color_enabled() : false;
    }
    
    logger() {
        use_console_log_strategy();
    }

    ~logger() {}

    class logmsg {
       public:
        logmsg(const log_level_t level, const std::string& filename, const int line, logger& self)
            : _level(level), _curr_time(getcurrtime()), _pid(getpid()), 
              _filename(filename), _line(line), _self(self) {
            std::stringstream ss;
            
            // Add color if enabled
            if (_self.is_color_enabled()) {
                ss << get_color_for_level(_level);
            }
            
            ss << '[' << _curr_time << ']'
               << '[' << loglevel2str(_level) << ']'
               << '[' << _pid << ']'
               << '[' << _filename << ']'
               << '[' << _line << ']'
               << "- ";
            
            _loginfo = ss.str();
        }

        ~logmsg() {
            // Reset color at the end of log message
            if (_self.is_color_enabled()) {
                _loginfo += Color::RESET;
            }
            if (_self._strategy) {
                _self._strategy->synclog(_loginfo);
            }
        }

        template <class T>
        logmsg& operator<<(const T& info) {
            std::stringstream ss;
            ss << info;
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
        // reference for outer
        logger& _self;
    };

    logmsg operator()(const log_level_t level, const std::string& filename, const int line) {
        return logmsg(level, filename, line, *this);
    }

   private:
    std::unique_ptr<log_strategy_vir> _strategy;
};

// __FILE__ filename without path
logger log;
#define LOG(level) log(level, __FILE__, __LINE__)

}  // namespace sym

#endif  // __LOG_HPP