#pragma once

#include <string>

#include "IniParse.hpp"

class Conf {
public:
    inline static std::string default_username;

    inline static std::string system_name;
    inline static std::string system_msg_sign;
    inline static std::string normal_msg_sign;

    inline static bool enable_log_debug;
};

/**
 * @brief init the conf
 */
inline void InitConf() {
    [[maybe_unused]] static bool initialized = [] {
        const auto& conf_instance = IniParser::GetInstance();
        Conf::default_username = conf_instance.Get("user", "default_username", "unnamed");

        Conf::system_name = conf_instance.Get("style", "system_name", "SYSTEM");
        Conf::system_msg_sign = conf_instance.Get("style", "system_msg_prompt", "$");
        Conf::normal_msg_sign = conf_instance.Get("style", "normal_msg_prompt", "#");

        Conf::enable_log_debug = conf_instance.Get("functions", "enable_log_debug", "false") == "true";
        return true;
    }();
}
