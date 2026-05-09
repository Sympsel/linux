#include "../../include/slog/Log.h"

// LogStrategyVirtual implementation
void sym::LogStrategyVirtual::SetColorEnabled(bool) {
}

bool sym::LogStrategyVirtual::IsColorEnabled() const {
    return false;
}

void sym::LogStrategyVirtual::Flush() {
}
