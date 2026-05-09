#include "../../include/slog/Log.h"

// CombinedLogStrategy implementation
sym::CombinedLogStrategy::CombinedLogStrategy(
    const std::string &logdir,
    const std::string &log_filename,
    bool color_enabled,
    bool daily_file)
    : _console(std::make_unique<ConsoleLogStrategy>(color_enabled)),
      _file(std::make_unique<FileLogStrategy>(logdir, log_filename, daily_file)) {
}

void sym::CombinedLogStrategy::SyncLog(const std::string &log_msg) {
    _console->SyncLog(log_msg);
    _file->SyncLog(log_msg);
}

void sym::CombinedLogStrategy::SetColorEnabled(const bool enabled) {
    _console->SetColorEnabled(enabled);
}

bool sym::CombinedLogStrategy::IsColorEnabled() const {
    return _console->IsColorEnabled();
}

void sym::CombinedLogStrategy::Flush() {
    _console->Flush();
    _file->Flush();
}