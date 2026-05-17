#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <utility>
#include <json/json.h>
#include "Food.hpp"
#include "Log.hpp"

namespace Sym {
    /**
     * @brief 配置项, 注册一个配置项分三步
     * 1. 添加配置项到结构体成员
     * 2. 在解析函数中读取配置文件
     * 3. 提供方括号重载获取整形配置值, 圆括号提供字符串配置值
     */
    struct ConfigItem {
        int width;
        int height;
        int def_len;
        int move_interval;
        int min_move_interval;
        int max_speed_level;
        int def_speed_level;
        int score_on_every_move;
        int max_hungry_time;

        std::vector<Food> food_list;

        ConfigItem() : width(), height(), def_len(),
                       move_interval(), min_move_interval(),
                       max_speed_level(), def_speed_level(),
                       score_on_every_move(), max_hungry_time() {
        }
    };

    class Conf {
    private:
        bool Parse() {
            std::ifstream file{_file};
            if (!file.is_open()) {
                return false;
            }
            auto json_str = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

            Json::Value root;
            Json::CharReaderBuilder reader;
            std::string errors;
            if (std::istringstream iss{json_str}; !Json::parseFromStream(reader, iss, &root, &errors)) {
                std::cout << "Error parsing JSON: " << errors << std::endl;
                return false;
            }

            _conf.width = root.get("width", 120).asInt();
            _conf.height = root.get("height", 80).asInt();
            _conf.def_len = root.get("def_len", 5).asInt();
            _conf.move_interval = root.get("move_interval", 200).asInt();
            _conf.min_move_interval = root.get("min_move_interval", 50).asInt();
            _conf.max_speed_level = root.get("max_speed_level", 9).asInt();
            _conf.def_speed_level = root.get("def_speed_level", 5).asInt();
            _conf.score_on_every_move = root.get("score_on_every_move", 1).asInt();
            _conf.max_hungry_time = root.get("max_hungry_time", 200).asInt();

            if (const Json::Value &food_array = root["food_list"]; food_array.isArray()) {
                for (const auto &food_item: food_array) {
                    Food food{};
                    food.score = food_item.get("score", 10).asInt();
                    food.percent = food_item.get("percent", 80).asInt();
                    food.duration_time = food_item.get("duration_time", 5).asInt();
                    food.hunger_restore = food_item.get("hunger_restore", 60).asInt();

                    std::string signal_str = food_item.get("signal", "*").asString();
                    food.signal = signal_str.empty() ? "●" : signal_str;

                    _conf.food_list.push_back(food);
                }
            }
            return true;
        }

    public:
        explicit Conf(std::string file = "")
            : _file(std::move(file)) {
            if (file.empty()) {
                _file = std::filesystem::canonical(
                    std::filesystem::current_path() / "config.json"
                );
            } else {
                _file = std::filesystem::canonical(file);
            }
            if (!std::filesystem::exists(_file)) {
                LOG_DEBUG() << "Loading config file: " << std::filesystem::absolute(_file);
                LOG_WARN() << "Config file not found";
                throw std::runtime_error("Config file not found.");
            }
        }

        /**
         * @brief 加载配置文件
         * @warning 仅在程序启动时调用, 再次调用不做任何处理
         */
        void Load() {
            static bool executed = false;
            if (!executed) {
                executed = true;
            }
            if (const bool loaded = Parse(); !loaded) {
                LOG_ERROR() << "Config file parse failed.";
            }
        }

        /**
         * @brief 获取整型配置项
         * @param key 配置项名
         * @return 配置项值
        */
        int operator[](const std::string &key) const {
            if (key == "width")
                return _conf.width;
            if (key == "height")
                return _conf.height;
            if (key == "def_len")
                return _conf.def_len;
            if (key == "move_interval")
                return _conf.move_interval;
            if (key == "min_move_interval")
                return _conf.min_move_interval;
            if (key == "max_speed_level")
                return _conf.max_speed_level;
            if (key == "def_speed_level")
                return _conf.def_speed_level;
            if (key == "score_on_every_move")
                return _conf.score_on_every_move;
            if (key == "max_hungry_time")
                return _conf.max_hungry_time;
            LOG_ERROR() << "Config key not found: " << key;
            return 0;
        }

        /**
         * @brief 获取字符串配置项
         * @param key 配置项名
         * @return 配置项值
         */
        std::string operator()(const std::string &key) const {
            // todo
            return {};
        }

        [[nodiscard]] const ConfigItem &GetConf() const {
            return _conf;
        }

        ~Conf() = default;

    private:
        std::string _file;
        ConfigItem _conf;
    };
}

extern Sym::Conf conf;
