#include <print>
#include <json/json.h>

// 数据序列化
bool serialize(const Json::Value &val, std::string &outStr) {
    Json::StreamWriterBuilder swb;
    swb["emitUTF8"] = true;
    const std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
    std::stringstream ss;
    if (sw->write(val, &ss)) {
        std::println("Json serialize failed!");
        return false;
    }
    outStr = ss.str();
    return true;
}

// 数据反序列化
bool unserialize(const std::string &jsonStr, Json::Value &outVal) {
    Json::CharReaderBuilder crb;
    std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
    std::string error;
    if (cr->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.size(), &outVal, &error)) {
        std::println("Json unserialize failed! errors: {}", error);
        return false;
    }
    return true;
}

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
    if (std::string stuStr; !serialize(student, stuStr)) {
        std::println("读取错误：{}", stuStr);
    } else {
        std::println("读取成功：{}", stuStr);
    }
    std::string jsonStr = R"({"name":"张三","age":18})";
    Json::Value v;
    unserialize(jsonStr, v);
    std::string outStr;
    serialize(v, outStr);
    std::println("{}", jsonStr);
    std::println("{}", outStr);
    return 0;
}
