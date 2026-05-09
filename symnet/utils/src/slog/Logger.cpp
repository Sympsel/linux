#include "../../include/slog/Log.h"

// Logger implementation
sym::Logger::Logger() {
    UseConsoleLog();
}

void sym::Logger::UseConsoleLog(bool color_enabled) {
    std::lock_guard locker(_strategy_lock);
    _strategy = std::make_unique<ConsoleLogStrategy>(color_enabled);
}

void sym::Logger::UseFileLogStrategy(
    const std::string &logdir,
    const std::string &log_filename,
    bool daily_file) {
    std::lock_guard locker(_strategy_lock);
    _strategy = std::make_unique<FileLogStrategy>(logdir, log_filename, daily_file);
}

void sym::Logger::UseCombinedLogStrategy(
    const std::string &log_dir,
    const std::string &log_filename,
    bool color_enabled,
    bool daily_file) {
    std::lock_guard locker(_strategy_lock);
    _strategy = std::make_unique<CombinedLogStrategy>(log_dir, log_filename, color_enabled, daily_file);
}

void sym::Logger::SetColorEnabled(const bool enabled) const {
    std::lock_guard locker(_strategy_lock);
    if (_strategy) {
        _strategy->SetColorEnabled(enabled);
    }
}

bool sym::Logger::IsColorEnabled() const {
    std::lock_guard locker(_strategy_lock);
    return _strategy ? _strategy->IsColorEnabled() : false;
}

void sym::Logger::Flush() const {
    std::lock_guard locker(_strategy_lock);
    if (_strategy) {
        _strategy->Flush();
    }
}

sym::Logger::LogMsg sym::Logger::operator()(log_level_t level,
                                            const std::string &filename,
                                            int line) {
    return {level, filename, line, *this};
}

void sym::Logger::SyncLog(const std::string &msg) const {
    std::lock_guard locker(_strategy_lock);
    if (_strategy) {
        _strategy->SyncLog(msg);
    }
}

sym::Logger &sym::GetLogger() {
    static Logger instance;
    return instance;
}
