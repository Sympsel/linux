#pragma once
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include "Log.hpp"

static std::string default_config_file;

/**
 * @brief you may handle the path of config file by yourself on it won't work
 */
class IniParser {
private:
    static void trim(std::string &str) {
        std::string tmp;
        for (const auto& c : str) {
            if (!std::isspace(c)) {
                tmp.push_back(c);
            }
        }
        str = tmp;
    }

    explicit IniParser(const std::string & file) {
        default_config_file = std::filesystem::canonical(
            std::filesystem::current_path() / ".." / "config.ini"
        );
        if (std::filesystem::exists(file)) {
            _file = file;
        } else {
            _file = default_config_file;
        }
        LOG_DEBUG() << "Load config file: " << _file;
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
     * @param file path of config file
     * @return an instance of IniParser
     */
    static IniParser& GetInstance(const std::string &file = "") {
        static IniParser instance{file};
        return instance;
    }

    bool reload() {
        return load();
    }

    std::string Get(const std::string &section, const std::string &key, const std::string &default_value = "") const {
        const auto key_name = section + "." + key;
        return _ini_map.find(key_name) != _ini_map.end() ? _ini_map.at(key_name) : default_value;
    }

private:
    std::unordered_map<std::string, std::string> _ini_map;
    std::string _file;
};
