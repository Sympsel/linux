#pragma once

#include <functional>

#include "HttpParser.hpp"
#include "HttpSerializer.hpp"
#include "HttpServer.hpp"
#include "../../utils/module/SymNet.h"

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    std::string status_code;
    std::string status_text;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

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

    static void BuildRequestToStream(const HttpRequest& req, std::string& req_stream) {

    }

public:
    using net_task_t = std::function<std::string(const std::string &)>;

    explicit HttpProtocol() = default;

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
    http_business_task_t
};
