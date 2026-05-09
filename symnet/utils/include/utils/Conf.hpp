#pragma once

#include <iostream>
#include "IniParse.hpp"

/**
 * @brief Centralized configuration manager for the symnet library.
 *
 * Provides static members for all configurable parameters across the library,
 * loaded from config.ini file via IniParser. All configuration values are
 * initialized automatically on first use.
 *
 * @example
 *
 * Configuration is autoloaded, just use it:
 * int backlog = Conf::network_backlog;

 * Or override before initialization:
 * Conf::network_backlog = 64;
 * Conf::threadpool_default_size = 8;
 */
class Conf {
public:
    // === Network Configuration ===
    /// Maximum number of pending connections in TCP listen queue
    inline static int network_backlog = 32;

    /// Default buffer size for UDP socket receive operations (in bytes)
    inline static int network_buffer_length = 1024;

    /// Default IP address for binding (0.0.0.0 means any interface)
    inline static std::string network_default_bind_ip = "0.0.0.0";

    // === Thread Configuration ===
    /// Enable debug logging for thread lifecycle events (start/stop)
    inline static bool thread_enable_log = false;

    /// Initial value for thread naming counter
    inline static int thread_name_counter_start = 0;

    // === Thread Pool Configuration ===
    /// Default number of worker threads in the thread pool
    inline static int threadpool_default_size = 5;

    /// Enable debug logging for thread pool task scheduling
    inline static bool threadpool_enable_log = false;

    // === Logger Configuration ===
    /// Default directory for log file storage
    inline static std::string log_directory = "./log/";

    /// Default log filename (empty means auto-generated with date)
    inline static std::string log_filename;

    /// Enable colored console output by default
    inline static bool log_color_enabled = true;

    /// Enable daily log file rotation by default
    inline static bool log_daily_rotation = true;

    // === Legacy Configuration (from other project) ===
    /// Default username for user identification
    inline static std::string default_username = "unnamed";

    /// System name for display purposes
    inline static std::string system_name = "SYSTEM";

    /// Message prompt symbol for system messages
    inline static std::string system_msg_sign = "$";

    /// Message prompt symbol for normal messages
    inline static std::string normal_msg_sign = "#";

    /// Enable debug level logging
    inline static bool enable_log_debug = false;
};

/**
 * @brief Initializes the configuration system by loading values from config.ini.
 *
 * This function is called automatically on first use via static initialization.
 * It reads all configuration values from the INI file and populates the Conf class
 * static members. If a value is not found in the config file, the default value is used.
 *
 * Boolean values must be exactly "true" or "false" (case-sensitive).
 * Other values like "True", "False", "0", "1" will be treated as false.
 *
 * @return true if initialization succeeds, false otherwise
 *
 * @example
 *
 * Auto-initialization (recommended)
 * InitConf(); // Called implicitly
 * Access configuration
 * int backlog = Conf::network_backlog;
 * bool debug = Conf::thread_enable_log;
 * Auto-initialization (recommended)
 * InitConf(); // Called implicitly
 * Access configuration
 * int backlog = Conf::network_backlog;
 * bool debug = Conf::thread_enable_log;
 */
inline void InitConf() {
    [[maybe_unused]] static bool initialized = [] {
        const auto &conf_instance = IniParser::GetInstance();

        // Network configuration
        {
            const auto &backlog_str = conf_instance.Get("network", "backlog", "32");
            try {
                Conf::network_backlog = std::stoi(backlog_str);
            } catch (...) {
                Conf::network_backlog = 32;
            }
        }

        {
            const auto &buffer_str = conf_instance.Get("network", "buffer_length", "1024");
            try {
                Conf::network_buffer_length = std::stoi(buffer_str);
            } catch (...) {
                Conf::network_buffer_length = 1024;
            }
        }

        Conf::network_default_bind_ip = conf_instance.Get("network", "default_bind_ip", "0.0.0.0");

        // Thread configuration
        Conf::thread_enable_log = conf_instance.Get("thread", "enable_log", "false") == "true";

        {
            const auto &counter_str = conf_instance.Get("thread", "name_counter_start", "0");
            try {
                Conf::thread_name_counter_start = std::stoi(counter_str);
            } catch (...) {
                Conf::thread_name_counter_start = 0;
            }
        }

        // Thread pool configuration
        {
            const auto &pool_size_str = conf_instance.Get("threadpool", "default_size", "5");
            try {
                Conf::threadpool_default_size = std::stoi(pool_size_str);
            } catch (...) {
                Conf::threadpool_default_size = 5;
            }
        }

        Conf::threadpool_enable_log = conf_instance.Get("threadpool", "enable_log", "false") == "true";

        // Logger configuration
        Conf::log_directory = conf_instance.Get("logger", "directory", "./log/");
        Conf::log_filename = conf_instance.Get("logger", "filename", "");
        Conf::log_color_enabled = conf_instance.Get("logger", "color_enabled", "true") == "true";
        Conf::log_daily_rotation = conf_instance.Get("logger", "daily_rotation", "true") == "true";

        // Legacy configuration
        Conf::default_username = conf_instance.Get("default", "default_username", "unnamed");
        Conf::system_name = conf_instance.Get("style", "system_name", "SYSTEM");
        Conf::system_msg_sign = conf_instance.Get("style", "system_msg_prompt", "$");
        Conf::normal_msg_sign = conf_instance.Get("style", "normal_msg_prompt", "#");
        Conf::enable_log_debug = conf_instance.Get("functions", "enable_log_debug", "false") == "true";

        return true;
    }();
}

/**
 * @brief Applies custom configuration values programmatically.
 *
 * Utility function to set multiple configuration parameters at once,
 * bypassing the config file. Useful for testing or runtime overrides.
 *
 * @param backlog TCP listen queue size (default: 32)
 * @param buffer_len UDP receive buffer size in bytes (default: 1024)
 * @param thread_count Default thread pool size (default: 5)
 * @param enable_colors Enable/disable colored log output (default: true)
 *
 * @example
 *
 * Override defaults before using the library
 * ConfigureNetworkAndThreads(64, 2048, 8, false);
 */
inline void ConfigureNetworkAndThreads(
    const int backlog = 32,
    const int buffer_len = 1024,
    const int thread_count = 5,
    const bool enable_colors = true) {
    Conf::network_backlog = backlog;
    Conf::network_buffer_length = buffer_len;
    Conf::threadpool_default_size = thread_count;
    Conf::log_color_enabled = enable_colors;
}

/**
 * @brief Prints current configuration to stdout for debugging.
 *
 * Displays all configuration values in a formatted manner.
 * Useful for verifying configuration loading and debugging.
 */
inline void PrintConf() {
    std::cout << "=== Current Configuration ===" << std::endl;
    std::cout << "[Network]" << std::endl;
    std::cout << "  backlog: " << Conf::network_backlog << std::endl;
    std::cout << "  buffer_length: " << Conf::network_buffer_length << std::endl;
    std::cout << "  default_bind_ip: " << Conf::network_default_bind_ip << std::endl;

    std::cout << "[Thread]" << std::endl;
    std::cout << "  enable_log: " << (Conf::thread_enable_log ? "true" : "false") << std::endl;
    std::cout << "  name_counter_start: " << Conf::thread_name_counter_start << std::endl;

    std::cout << "[ThreadPool]" << std::endl;
    std::cout << "  default_size: " << Conf::threadpool_default_size << std::endl;
    std::cout << "  enable_log: " << (Conf::threadpool_enable_log ? "true" : "false") << std::endl;

    std::cout << "[Logger]" << std::endl;
    std::cout << "  directory: " << Conf::log_directory << std::endl;
    std::cout << "  filename: " << (Conf::log_filename.empty() ? "(auto)" : Conf::log_filename) << std::endl;
    std::cout << "  color_enabled: " << (Conf::log_color_enabled ? "true" : "false") << std::endl;
    std::cout << "  daily_rotation: " << (Conf::log_daily_rotation ? "true" : "false") << std::endl;

    std::cout << "[Legacy]" << std::endl;
    std::cout << "  default_username: " << Conf::default_username << std::endl;
    std::cout << "  system_name: " << Conf::system_name << std::endl;
    std::cout << "  enable_log_debug: " << (Conf::enable_log_debug ? "true" : "false") << std::endl;
    std::cout << "=============================" << std::endl;
}
