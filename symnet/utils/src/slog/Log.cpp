#include "../../include/slog/Log.h"
#include <filesystem>
#include <sstream>
#include <iostream>

// ... existing code ...

std::string sym::GetCurrTime() {
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

std::string sym::GetCurrDate() {
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

std::string sym::LogLevelToStr(const log_level_t &level) {
    switch (level) {
        case log_level_t::DEBUG: return "DEBUG";
        case log_level_t::INFO: return "INFO";
        case log_level_t::WARNING: return "WARNING";
        case log_level_t::ERROR: return "ERROR";
        case log_level_t::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string sym::GetColorForLevel(const log_level_t &level) {
    switch (level) {
        case log_level_t::DEBUG: return Color::BOLD_CYAN;
        case log_level_t::INFO: return Color::BOLD_BLUE;
        case log_level_t::WARNING: return Color::BOLD_YELLOW;
        case log_level_t::ERROR: return Color::BOLD_RED;
        case log_level_t::FATAL: return Color::BOLD_MAGENTA;
        default: return Color::RESET;
    }
}



