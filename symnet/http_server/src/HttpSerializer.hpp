#pragma once

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include "DataType.hpp"
#include "../../utils/module/SymNet.h"


class HttpSerializer : public ISerializer<HttpRequest, HttpResponse> {
public:
    [[nodiscard]] std::string SerializeRequest(const HttpRequest &req) const override {
        Json::Value root;
        root["method"] = req.method;
        root["path"] = req.path;
        root["version"] = req.version;
        Json::Value headers;
        for (const auto &[key, value]: req.headers) {
            headers[key] = value;
        }
        root["headers"] = headers;
        root["body"] = req.body;
        return Write(root);
    }

    bool DeserializeRequest(const std::string &str, HttpRequest &out) const override {
        Json::Value root;
        if (!Parse(str, root)) return false;
        out.method = root["method"].asString();
        out.path = root["path"].asString();
        out.version = root["version"].asString();
        for (const auto & key : root["headers"].getMemberNames()) {
            out.headers[key] = root["headers"][key].asString();
        }
        out.body = root["body"].asString();
        return true;
    }

    [[nodiscard]] std::string SerializeResponse(const HttpResponse &resp) const override {
        Json::Value root;
        root["status_code"] = resp.status_code;
        root["status_text"] = resp.status_text;
        Json::Value headers;
        for (const auto &[key, values]: resp.headers) {
            headers[key] = values;
        }
        root["headers"] = headers;
        root["body"] = resp.body;
        return Write(root);
    }

    bool DeserializeResponse(const std::string &str, HttpResponse &out) const override {
        Json::Value root;
        if (!Parse(str, root)) return false;
        out.status_code = root["status_code"].asString();
        out.status_text = root["status_text"].asString();
        for (const auto & key : root["headers"].getMemberNames()) {
            out.headers[key] = root["headers"][key].asString();
        }
        out.body = root["body"].asString();
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
