#pragma once
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include "Log.hpp"

/**
 * @brief INI configuration file parser with singleton pattern.
 *
 * Provides thread-safe access to configuration values from INI files.
 * The default config file path is automatically determined based on the location
 * of this header file, ensuring reliable path resolution regardless of the
 * working directory.
 */
class IniParser {
private:
    static void trim(std::string &str) {
        std::string tmp;
        for (const auto &c: str) {
            if (!std::isspace(c)) {
                tmp.push_back(c);
            }
        }
        str = tmp;
    }

    /**
     * @brief Resolves the default config file path relative to this header file.
     *
     * Assumes config.ini is located at: <header_dir>/../../config.ini
     * (i.e., two levels up from this header's directory)
     *
     * @return Absolute path to config.ini
     */
    static std::string GetDefaultConfigPath() {
        // __FILE__ gives us the path to this header file
        // e.g., /path/to/symnet/lib/include/utils/IniParse.hpp
        const auto header_path = std::filesystem::path(__FILE__);
        const std::filesystem::path header_dir = header_path.parent_path();
        // Navigate up to lib directory: utils -> include -> lib
        const std::filesystem::path lib_dir = header_dir.parent_path().parent_path();
        // Config file is in lib/config.ini
        const std::filesystem::path config_path = lib_dir / "config.ini";

        try {
            return std::filesystem::canonical(config_path).string();
        } catch (const std::filesystem::filesystem_error &e) {
            LOG_WARN() << "Failed to resolve config path: " << e.what();
            return config_path.string();
        }
    }

    explicit IniParser(const std::string &file) {
        if (!file.empty()) {
            _file = file;
        } else {
            // Use auto-detected path based on __FILE__
            _file = GetDefaultConfigPath();
        }

        // Convert to absolute path for consistency
        try {
            if (std::filesystem::exists(_file)) {
                _file = std::filesystem::canonical(_file).string();
            }
        } catch (const std::filesystem::filesystem_error &e) {
            LOG_WARN() << "Failed to canonicalize config path: " << e.what();
        }
        load();
    }

    bool load() {
        if (!std::filesystem::exists(_file)) {
            LOG_WARN() << "Config file not exists: " << _file;
            return false;
        }
        std::ifstream config{_file};
        std::string line, current_section;
        while (std::getline(config, line)) {
            trim(line);
            if (line.empty() || line[0] == ';' || line[0] == '#') continue;
            if (line[0] == '[') {
                current_section = line.substr(1, line.size() - 2);
                trim(current_section);
            } else if (const auto pos = line.find('='); pos != std::string::npos) {
                auto key = line.substr(0, pos);
                auto key_with_sec = current_section;
                key_with_sec.append("." + key);
                auto value = line.substr(pos + 1);
                trim(key_with_sec);
                trim(value);
                _ini_map[key_with_sec] = value;
            }
        }
        return true;
    }

public:
    IniParser(const IniParser &) = delete;

    IniParser &operator=(const IniParser &) = delete;

    IniParser(IniParser &&) = delete;

    IniParser &operator=(IniParser &&) = delete;

    /**
     * @brief Gets the singleton instance of IniParser.
     * @param file Path to config file (optional). If empty, auto-detects based on header location.
     * @return Reference to the singleton IniParser instance
     *
     * @example
     *
     * Auto-detect config.ini relative to IniParse.hpp
     * auto& parser = IniParser::GetInstance();
     *
     * Use custom path
     * auto& parser = IniParser::GetInstance("/custom/path/config.ini");
     */
    static IniParser &GetInstance(const std::string &file = "") {
        static IniParser instance{file};
        return instance;
    }

    /**
     * @brief Reloads the configuration file.
     * @return true if reload succeeds, false otherwise
     */
    bool reload() {
        return load();
    }

    /**
     * @brief Gets a configuration value.
     * @param section Section name in INI file (e.g., "network")
     * @param key Key name within the section (e.g., "backlog")
     * @param default_value Default value if key not found
     * @return Configuration value or default_value if not found
     *
     * @example
     *
     * std::string ip = parser.Get("network", "default_bind_ip", "0.0.0.0");
     * int backlog = std::stoi(parser.Get("network", "backlog", "32"));
     * bool debug = parser.Get("thread", "enable_log", "false") == "true";
     *
     */
    std::string Get(const std::string &section, const std::string &key, const std::string &default_value = "") const {
        const auto key_name = section + "." + key;
        return _ini_map.find(key_name) != _ini_map.end() ? _ini_map.at(key_name) : default_value;
    }

    /**
     * @brief Gets the current config file path being used.
     * @return Absolute path to the loaded config file
     */
    std::string GetConfigPath() const {
        return _file;
    }

private:
    std::unordered_map<std::string, std::string> _ini_map;
    std::string _file;
};
