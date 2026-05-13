#pragma once

#include <functional>

#include "HttpParser.hpp"
#include "HttpSerializer.hpp"
#include "../../utils/module/SymNet.h"

template<class TaskType>
class HttpProtocol {
private:
    static HttpResponse MakeErrorResponse(const std::string &code, const std::string &text) {
        HttpResponse resp;
        resp.status_code = code;
        resp.status_text = text;
        resp.headers["Content-Type"] = "text/plain";
        resp.body = text;
        return resp;
    }

public:
    using net_task_t = std::function<std::string(const std::string &)>;

    explicit HttpProtocol(const TaskType &task)
        : _task(task),
          _serializer(std::make_unique<HttpSerializer>()) {
    }

    net_task_t GetHandler() {
        return [this](const std::string &http_req_str) -> std::string {
            // 解析标准 HTTP 请求
            HttpRequest http_req;
            if (!HttpParser::ParseRequest(http_req_str, http_req)) {
                LOG_ERROR() << "parse http request failed";
                return HttpParser::BuildResponse(MakeErrorResponse("400", "Bad Request"));
            }
            LOG_INFO() << "parsed HTTP request: " << http_req.method << " " << http_req.path;

            const HttpResponse http_resp = _task(http_req);

            // 构建标准 HTTP 响应
            return HttpParser::BuildResponse(http_resp);
        };
    }

    ~HttpProtocol() = default;

private:
    TaskType _task;
    std::unique_ptr<HttpSerializer> _serializer;
};
