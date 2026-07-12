#pragma once

#include <json/json.h>
#include <print>

class JsonTrans {
public:
    // 数据序列化
    static bool serialize(const Json::Value &val, std::string &outStr) {
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
    static bool unserialize(const std::string &jsonStr, Json::Value &outVal) {
        const Json::CharReaderBuilder crb;
        const std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
        std::string error;
        if (cr->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.size(), &outVal, &error)) {
            std::println("Json unserialize failed! errors: {}", error);
            return false;
        }
        return true;
    }

};