#include <filesystem>
#include <iostream>

#include "../../include/slog/Log.h"

// FileLogStrategy implementation
sym::FileLogStrategy::FileLogStrategy(
    std::string logdir,
    std::string log_filename,
    bool daily_file)
    : _logdir(std::move(logdir)),
      _base_filename(std::move(log_filename)),
      _daily_file(daily_file),
      _current_date(GetCurrDate()) {
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

sym::FileLogStrategy::~FileLogStrategy() {
    std::lock_guard locker(_lock);
    if (_ofs.is_open()) {
        _ofs.close();
    }
}

void sym::FileLogStrategy::SyncLog(const std::string &log_msg) {
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

bool sym::FileLogStrategy::IsColorEnabled() const {
    return false;
}

void sym::FileLogStrategy::Flush() {
    std::lock_guard locker(_lock);
    if (_ofs.is_open()) {
        _ofs.flush();
    }
}

std::string sym::FileLogStrategy::GetCurrentLogfile() const {
    std::lock_guard locker(_lock);
    return _logdir + GetFilenameHelper();
}

std::string sym::FileLogStrategy::RemoveColorCodes(const std::string &msg) {
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

std::string sym::FileLogStrategy::GetFilenameHelper() const {
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

void sym::FileLogStrategy::OpenLogFile() {
    if (_ofs.is_open()) {
        _ofs.close();
    }
    const std::string filepath = _logdir + GetFilenameHelper();
    _ofs.open(filepath, std::ios::app);
    if (!_ofs.is_open()) {
        std::cerr << "[Logger Error] Failed to open log file: " << filepath << std::endl;
    }
}

