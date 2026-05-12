#pragma once

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include "../../utils/module/SymNet.h"

// todo replace int/int
class HttpSerializer : public ISerializer<int, int> {
public:
    std::string SerializeRequest(const int &req) const override {
        Json::Value root;
        root["x"] = 1;
        return Write(root);
    }

    bool DeserializeRequest(const std::string &str, int &out) const override {
        Json::Value root;
        if (!Parse(str, root)) return false;
        out = root["x"].asInt();
        return true;
    }

    std::string SerializeResponse(const int &resp) const override {
        Json::Value root;
        root["x"] = 1;
        return Write(root);
    }

    bool DeserializeResponse(const std::string &str, int &out) const override {
        Json::Value root;
        if (!Parse(str, root)) return false;
        out = root["x"].asInt();
        return true;
    }

private:
    static std::string Write(const Json::Value &root) {
        const Json::StreamWriterBuilder builder;
        return Json::writeString(builder, root);
    }

    static bool Parse(const std::string &str, Json::Value &out) {
        const Json::CharReaderBuilder builder;
        JSONCPP_STRING err;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        return reader->parse(str.c_str(), str.c_str() + str.size(), &out, &err);
    }
};
