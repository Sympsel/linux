#include <iostream>

#include "../../include/slog/Log.h"

// ConsoleLogStrategy implementation
sym::ConsoleLogStrategy::ConsoleLogStrategy(const bool color_enabled)
    : _color_enabled(color_enabled) {
}

void sym::ConsoleLogStrategy::SyncLog(const std::string &log_msg) {
    std::lock_guard locker(_lock);
    std::cout << log_msg << std::endl;
}

void sym::ConsoleLogStrategy::SetColorEnabled(const bool enabled) {
    std::lock_guard locker(_lock);
    _color_enabled = enabled;
}

bool sym::ConsoleLogStrategy::IsColorEnabled() const {
    return _color_enabled;
}

void sym::ConsoleLogStrategy::Flush() {
    std::lock_guard locker(_lock);
    std::cout.flush();
}
