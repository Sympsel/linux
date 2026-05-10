#pragma once

#include "../slog/Log.h"

/**
 * SerializerType 必须实现以下接口:
 * - using RequestType = ...
 * - using ResponseType = ...
 * - bool DeserializeRequest(const string&, RequestType&)
 * - string SerializeResponse(const ResponseType&)
 */
template<class TaskType, class SerializerType>
class ProtoCol {
public:
    explicit ProtoCol(const TaskType &task, SerializerType serializer)
        : _task(task), _serializer(std::move(serializer)) {
    }

    [[nodiscard]] std::string Package(const std::string &json_str) const {
        const size_t len = json_str.size();
        return std::to_string(len) + _sep + json_str + _sep;
    }

    ssize_t Unpackage(std::string &stream_str, std::string &json_str_to_fill) const {
        if (const auto pos = stream_str.find(_sep); pos == std::string::npos) {
            return 0;
        } else {
            const std::string pack_len_str = stream_str.substr(0, pos);
            for (const auto &c: pack_len_str) {
                if (!isdigit(c)) {
                    LOG_ERROR() << "invalid pack format: [package_len: " << pack_len_str << "]";
                    return -1;
                }
            }
            const ssize_t pack_len = std::stoi(pack_len_str);
            const ssize_t total_len = pack_len + pack_len_str.size() + _sep.size() * 2;
            if (stream_str.size() < total_len) {
                return 0;
            }
            json_str_to_fill = stream_str.substr(pos + _sep.size(), pack_len);
            stream_str.erase(0, total_len);
            return pack_len;
        }
    }

    [[nodiscard]] std::string HandleRequest(const std::string &stream_str) const {
        std::string json_str;

        // 解包
        if (const ssize_t n = Unpackage(const_cast<std::string &>(stream_str), json_str); n <= 0) {
            return {};
        }

        // 反序列化请求
        typename SerializerType::RequestType request;
        if (!_serializer.DeserializeRequest(json_str, request)) {
            return {};
        }

        // 处理请求 (调用业务层)
        typename SerializerType::ResponseType response = _task(request);

        // 序列化响应
        const std::string result = _serializer.SerializeResponse(response);

        // 打包并返回
        return Package(result);
    }

private:
    std::string _sep = "\r\n";
    TaskType _task;
    SerializerType _serializer;
};

