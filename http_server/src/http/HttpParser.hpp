#pragma once

#include <algorithm>
#include <memory>
#include <string>

#include "../utils/Buffer.hpp"
#include "HttpData.hpp"

class HttpParser {
public:
    HttpParser() = delete;

    enum ParseState {
        PARSE_REQUEST_LINE,
        PARSE_HEADERS,
        PARSE_BODY,
        PARSE_DONE
    };

    // 反序列化 - 采用状态机模式
    static bool parse(Buffer &buffer, HttpRequest &request) {
        using namespace http_v;
        ParseState state = PARSE_REQUEST_LINE;
        std::string line;
        size_t cont_len{};

        while (state != PARSE_DONE && buffer.getReadableBytes() > 0) {
            const char *crlf = buffer.findCRLF();
            if (!crlf) break;
            const size_t line_len = crlf - buffer.peek();

            line.assign(buffer.peek(), line_len);
            buffer.retrieve(line_len + line_spilt.size());

            switch (state) {
                case PARSE_REQUEST_LINE:
                    // 从请求行中解析出请求方法、路径和版本
                    if (!parseRequestLine(line, request)) {
                        return false;
                    }
                    state = PARSE_HEADERS;
                    break;
                case PARSE_HEADERS:
                    if (line.empty()) {
                        // 解析到空行说明头部结束
                        cont_len = getContentLength(request);
                        if (cont_len > 0) {
                            state = PARSE_BODY;
                        } else {
                            state = PARSE_DONE;
                        }
                    } else {
                        parseHeader(line, request);
                    }
                    break;
                case PARSE_BODY:
                    if (buffer.getReadableBytes() >= cont_len) {
                        request.body.assign(buffer.peek(), cont_len);
                        buffer.retrieve(cont_len);
                        state = PARSE_DONE;
                    }
                    break;
                case PARSE_DONE:
                default: break;
            }
        }
        return state == PARSE_DONE;
    }

private:
    // 解析请求行
    static bool parseRequestLine(const std::string &line, HttpRequest &request) {
        using namespace http_v;
        const size_t first_space = line.find(space);
        const size_t second_space = line.find(space, first_space + 1);
        if (first_space == std::string::npos || second_space == std::string::npos) {
            return false;
        }

        const std::string method_str = line.substr(0, first_space);
        request.method = getMethod(method_str);
        request.path = line.substr(first_space + space.size(),
                                   second_space - first_space - space.size());
        request.version = line.substr(second_space + space.size());
        return true;
    }

    static void parseHeader(const std::string &line, HttpRequest &request) {
        using namespace http_v;
        const size_t colon_pos = line.find(kv_split);
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + kv_split.size());
            trim(key);
            trim(value);
            request.setHeader(key, value);
        }
    }

    static size_t getContentLength(const HttpRequest &request) {
        const std::string len_str = request.getHeader("Content-Length");
        return len_str.empty() ? 0 : std::stoul(len_str);
    }

    static HttpMethod getMethod(const std::string &method_str) {
        if (method_str == "GET") return HttpMethod::GET;
        if (method_str == "POST") return HttpMethod::POST;
        if (method_str == "PUT") return HttpMethod::PUT;
        if (method_str == "DELETE") return HttpMethod::DELETE;
        return HttpMethod::UNKNOWN;
    }

    static void trim(std::string &str) {
        auto begin = str.begin();
        auto end = std::prev(str.end());
        while (begin < end && std::isspace(*begin)) {
            ++begin;
        }
        while (begin < end && std::isspace(*end)) {
            --end;
        }
        str = std::string(begin, end + 1);
    }
};
