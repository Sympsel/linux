#include <iostream>

#include "utils/JsonTrans.hpp"

int main() {
    const char *name = "小米";
    int age = 19;
    const char *sex = "男";
    float scores[3] = {100, 10.01, 1001};

    Json::Value student;
    student["name"] = name;
    student["age"] = age;
    student["sex"] = sex;
    for (const auto &score: scores) {
        student["scores"].append(score);
    }
    if (std::string stuStr; !JsonTrans::serialize(student, stuStr)) {
        std::println("读取错误：{}", stuStr);
    } else {
        std::println("读取成功：{}", stuStr);
    }
    std::string jsonStr = R"({"name":"张三","age":18})";
    Json::Value v;
    JsonTrans::unserialize(jsonStr, v);
    std::string outStr;
    JsonTrans::serialize(v, outStr);
    std::println("{}", jsonStr);
    std::println("{}", outStr);
    return 0;
}
