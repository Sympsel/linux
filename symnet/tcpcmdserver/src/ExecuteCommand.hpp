#pragma once

#include <functional>
#include <unordered_set>

#include "slog/Log.h"

using exec_cmd_t = std::function<std::string(const std::string&)>;

class ExecuteCommandServer {
private:
    void DefaultWhiteList() {
        _white_list.emplace("ls");
    }

public:
    /**
     * @brief 命令白名单
     *
     * @param cmd 需要执行的命令
     * @return 是否在白名单中
     */
    bool IsWhiteList(const std::string& cmd) {
        return _white_list.find(cmd) != _white_list.end();
    }

    /**
     * @brief 添加命令白名单
     *
     * @param cmd 需要添加的命令
     */
    void AddToWhiteList(const std::string& cmd) {
        _white_list.emplace(cmd);
    }

    std::string Execute(const std::string& cmd) {
        if (!IsWhiteList(cmd)) {
            return "command not in white list\n";
        }
        FILE* fp = popen(cmd.c_str(), "r");
        if (fp == nullptr) {
            LOG_ERROR() << "exec error: " << cmd;
            return "execute fault\n";
        }

        char buffer[1024];

        std::stringstream result;
        while (fgets(buffer, sizeof buffer, fp) != nullptr) {
            result << buffer;
        }
        result << "\n";
        fclose(fp);
        return result.str();
    }

    void Init() {
        DefaultWhiteList();
    }
private:
    std::unordered_set<std::string> _white_list;
};
