#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include <print>

#include "Buffer.hpp"

enum class HttpMethod {
    GET, POST, PUT, DELETE, HEAD, UNKNOWN
};

namespace httpValue {
    inline constexpr std::string lineSpilt = "\r\n";
    inline constexpr std::string kvSplit = ": ";
    inline constexpr std::string version = "HTTP/1.1";
    inline constexpr std::string space = " ";
    inline constexpr std::string trimList = " \r\t\n";
}

class HttpUtils {
public:

};

inline std::string &trim(std::string &str) {
    const auto beginPos = str.find_first_not_of(httpValue::trimList);
    if (beginPos == std::string::npos) {
        str.clear();
        return str;
    }

    const auto endPos = str.find_last_not_of(httpValue::trimList);
    str = str.substr(beginPos, endPos - beginPos + 1);
    return str;
}

inline std::string trimmed(const std::string &str) {
    std::string result = str;
    return trim(result);
}

struct HttpRequest {
    HttpMethod method = HttpMethod::UNKNOWN;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> args;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string getHeader(const std::string &key) const {
        return headers.contains(key) ? headers.at(key) : "";
    }

    std::string getArg(const std::string &key) const {
        return args.contains(key) ? args.at(key) : "";
    }

    std::string methodToString() const {
        return methodToString(method);
    }

    void print() const {
        std::println("============= Request ==============");
        std::print("Method: {}\n, Path: {}\n, Version: {}\n"
                   , methodToString(), path, version);
        std::println("Headers:");
        for (const auto &[key, value]: headers) {
            std::println("\t{}{}{}", key, httpValue::kvSplit, value);
        }
        if (!args.empty()) {
            std::println("Args:\n");
            for (const auto &[key, value]: args) {
                std::println("\t{}{}{}", key, httpValue::kvSplit, value);
            }
        }
        if (body.size() > 50) {
            std::println("Body: {} ...", body.substr(0, 50));
        } else {
            std::println("Body: {}", body);
        }
    }

    static std::string methodToString(const HttpMethod method) {
        switch (method) {
            case HttpMethod::GET: return "GET";
            case HttpMethod::POST: return "POST";
            case HttpMethod::PUT: return "PUT";
            case HttpMethod::DELETE: return "DELETE";
            case HttpMethod::HEAD: return "HEAD";
            default: return "UNKNOWN";
        }
    }

    static HttpMethod getMethod(const std::string &method_str) {
        if (method_str == "GET") return HttpMethod::GET;
        if (method_str == "POST") return HttpMethod::POST;
        if (method_str == "PUT") return HttpMethod::PUT;
        if (method_str == "DELETE") return HttpMethod::DELETE;
        return HttpMethod::UNKNOWN;
    }

    static std::optional<HttpRequest> parse(Buffer &buffer, HttpRequest &httpReq) {
        using namespace httpValue;
        ParseState state = REQUEST_LINE;
        std::string line;
        size_t bodyLen{};

        while (state != DONE && buffer.getReadableSize() > 0) {
            const char *crlf = buffer.findCRLF();
            if (crlf == nullptr) break;
            const size_t lineLen = crlf - buffer.peek();
            line.assign(buffer.peek(), lineLen);
            buffer.retrieve(lineLen + lineSpilt.size());

            switch (state) {
                case REQUEST_LINE:
                    if (!parseQueryLine(line, httpReq)) {
                        return std::nullopt;
                    }
                    state = HEADERS;
                    break;
                case HEADERS:
                    // 解析到空行说明头部结束
                    if (line.empty()) {
                        bodyLen = getContentLength(httpReq);
                        if (bodyLen == 0) {
                            state = DONE;
                        } else {
                            state = BODY;
                        }
                    } else {
                        parseHeader(line, httpReq);
                    }
                    break;
                case BODY:
                    if (buffer.getReadableSize() >= bodyLen) {
                        httpReq.body.assign(buffer.peek(), bodyLen);
                        buffer.retrieve(bodyLen);
                        state = DONE;
                    }
                    break;
            }
        }
        return state == DONE ? std::optional(httpReq) : std::nullopt;
    }

    static void parseQueryParams(HttpRequest &httpReq) {
        const auto pos = httpReq.path.find('?');
        if (pos == std::string::npos) {
            return;
        }
        const std::string query = httpReq.path.substr(pos + 1);
        std::istringstream iss(query);
        std::string pair;
        while (std::getline(iss, pair, '&')) {
            auto equalPos = pair.find('=');
            if (equalPos != std::string::npos) {
                std::string key = pair.substr(0, equalPos);
                std::string value = pair.substr(equalPos + 1);
                httpReq.args[::trim(key)] = ::trim(value);
            }
        }
    }

    void addHeaderItem(const std::string &key, const std::string &value) {
        headers[key] = value;
    }

private:
    enum ParseState {
        REQUEST_LINE,
        HEADERS,
        BODY,
        DONE
    };

    void parseQueryParams() {
        parseQueryParams(*this);
    }

    // 解析请求行
    static bool parseQueryLine(const std::string &line, HttpRequest &request) {
        using namespace httpValue;
        const size_t firstSpace = line.find(space);
        const size_t secondSpace = line.find(space, firstSpace + 1);
        if (firstSpace == std::string::npos || secondSpace == std::string::npos) {
            return false;
        }

        const std::string method_str = line.substr(0, firstSpace);
        request.method = getMethod(method_str);
        request.path = line.substr(firstSpace + space.size(),
                                   secondSpace - firstSpace - space.size());
        request.version = line.substr(secondSpace + space.size());
        return true;
    }

    static void parseHeader(const std::string &line, HttpRequest &request) {
        using namespace httpValue;
        if (const size_t colonPos = line.find(kvSplit);
            colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + kvSplit.size());
            ::trim(key);
            ::trim(value);
            request.addHeaderItem(key, value);
        }
    }

    static size_t getContentLength(const HttpRequest &request) {
        const std::string len_str = request.getHeader("Content-Length");
        return len_str.empty() ? 0 : std::stoul(len_str);
    }
};

struct HttpResponse {
    std::string statusCode;
    std::string statusText;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    void print() const {
        std::println("============= Response ==============");
        std::print("Status Code: {}\nStatus Text: {}\n",
                   statusCode, statusCode);
        std::println("Headers:");
        for (const auto &[key, value]: headers) {
            std::println("\t{}{}{}", key, httpValue::kvSplit, value);
        }
        if (body.size() > 50) {
            std::println("Body: {} ...", body.substr(0, 50));
        } else {
            std::println("Body: {}", body);
        }
    }

    std::string serialize() const {
        using namespace httpValue;
        // 状态行
        std::string result = std::format("{}{}{}{}{}{}",
                                         version, space,
                                         statusCode, space,
                                         statusText, lineSpilt
        );
        // 响应头
        for (const auto &[key, value]: headers) {
            result.append(std::format("{}{}{}{}",
                                      key, kvSplit, value, lineSpilt
            ));
        }
        if (!headers.contains("Content-Length") && !body.empty()) {
            result.append(std::format(
                "{}{}{}{}",
                "Content-Length", kvSplit, body.size(), lineSpilt
            ));
        }
        // 空行及响应体
        result.append(lineSpilt);
        result.append(body);
        return result;
    }

    // HttpResponse deserialize(std::string &responseStr) {
    // }
};
