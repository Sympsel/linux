#include <unistd.h>

#include "../../include/slog/Log.h"

sym::Logger::LogMsg::LogMsg(
    const log_level_t level,
    std::string filename,
    const int line,
    Logger &self)
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
            << LogLevelToStr(_level) << " | "
            << "PID:" << _pid << " | "
            << _filename << ':' << _line << "] ";

    _log_info = ss.str();
}

sym::Logger::LogMsg::~LogMsg() {
    if (_self.IsColorEnabled()) {
        _log_info += Color::RESET;
    }

    _self.SyncLog(_log_info);
}
