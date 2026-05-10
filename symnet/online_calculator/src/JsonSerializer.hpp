#pragma once

#include <json/json.h>

#include "CalcTypes.hpp"

#include "../../utils/module/SymNet.h"


class JsonSerializer : public ISerializer<CalcRequest, CalcResponse> {
public:
    [[nodiscard]] std::string SerializeRequest(const CalcRequest& req) const override {
        Json::Value root;
        root["x"] = req.x;
        root["y"] = req.y;
        root["op"] = std::string(1, req.oper);
        return Write(root);
    }

    bool DeserializeRequest(const std::string& str, CalcRequest& out) const override {
        Json::Value root;
        if (!Parse(str, root)) return false;
        out.x = root["x"].asInt();
        out.y = root["y"].asInt();
        out.oper = root["op"].asString()[0];
        return true;
    }

    [[nodiscard]] std::string SerializeResponse(const CalcResponse& resp) const override {
        Json::Value root;
        root["result"] = resp.result;
        root["status"] = static_cast<int>(resp.status);
        return Write(root);
    }

    bool DeserializeResponse(const std::string& str, CalcResponse& out) const override {
        Json::Value root;
        if (!Parse(str, root)) return false;
        out.result = root["result"].asInt();
        out.status = static_cast<CalcStatus>(root["status"].asInt());
        return true;
    }

private:
    static std::string Write(const Json::Value& root) {
        const Json::StreamWriterBuilder builder;
        return Json::writeString(builder, root);
    }

    static bool Parse(const std::string& str, Json::Value& out) {
        const Json::CharReaderBuilder builder;
        std::string errs;
        std::istringstream iss(str);
        return Json::parseFromStream(builder, iss, &out, &errs);
    }
};