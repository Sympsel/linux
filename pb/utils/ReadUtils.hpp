#pragma once

#include <iostream>
#include <limits>
#include <string>

class ReadUtils {
public:
    // 忽略直到指定分隔符（默认换行符）
    static void ignoreUntil(const char delim = '\n') {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), delim);
    }

    // 清除 cin 错误状态并忽略剩余输入
    static void clearError() {
        if (std::cin.fail()) {
            std::cin.clear();  // 清除错误标志
            ignoreUntil();     // 忽略错误的输入
        }
    }

    // 读取一行
    static bool readLine(std::string& line) {
        return static_cast<bool>(std::getline(std::cin, line));
    }

    // 安全读取整数（带错误处理）
    static bool readInt(int& value) {
        if (std::cin >> value) {
            return true;
        }
        clearError();  // 如果失败，清除错误状态
        return false;
    }

    // 读取整数直到成功
    static int readIntWithRetry() {
        int value;
        while (true) {
            if (readInt(value)) {
                return value;
            }
            std::print("输入无效，请输入数字: ");
        }
    }
};
;