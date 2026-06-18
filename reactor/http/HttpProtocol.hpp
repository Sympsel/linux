#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include <print>

#include "Buffer.hpp"

enum class HttpMethod {
    GET, POST, PUT, DELETE, HEAD, UNKNOWN
};

inline constexpr auto VERSION = "HTTP/1.1";

struct HttpReq {
    HttpMethod method = HttpMethod::UNKNOWN;
    std::string path;
    std::string version = ::VERSION;
    std::unordered_map<std::string, std::string> args;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct HttpResp {
    std::string version = ::VERSION;
    int statusCode;
    std::string statusText;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class HttpUtils {
public:
    static std::string lineSpilt;
    static std::string kvSplit;
    static std::string trimList;

private:
    enum ParseState {
        REQUEST_LINE,
        STATUS_LINE,
        HEADERS,
        BODY,
        DONE
    };

    static bool parseQueryLine(const std::string &line, HttpReq &request) {
        const size_t firstSpace = line.find(' ');
        const size_t secondSpace = line.find(' ', firstSpace + 1);
        if (firstSpace == std::string::npos || secondSpace == std::string::npos) {
            return false;
        }

        const std::string methodStr = line.substr(0, firstSpace);
        request.method = getMethod(methodStr);
        request.path = line.substr(firstSpace + 1,
                                   secondSpace - firstSpace - 1);
        request.version = line.substr(secondSpace + 1);
        return true;
    }

    static bool parseStatusLine(const std::string &line, HttpResp &httpResp) {
        const size_t firstSpace = line.find(' ');
        const size_t secondSpace = line.find(' ', firstSpace + 1);
        if (firstSpace == std::string::npos || secondSpace == std::string::npos) {
            return false;
        }
        try {
            httpResp.version = line.substr(0, firstSpace);
            const std::string statusCodeStr = line.substr(firstSpace + 1,
                                                          secondSpace - firstSpace - 1);
            httpResp.statusCode = std::stoi(statusCodeStr);
            httpResp.statusText = line.substr(secondSpace + 1);
        } catch (const std::exception &) {
            return false;
        }
        return true;
    }

    static void parseHeader(const std::string &line, std::unordered_map<std::string, std::string> &headers) {
        if (const size_t colonPos = line.find(kvSplit);
            colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + kvSplit.size());
            trim(key);
            trim(value);
            addHeaderItem(headers, key, value);
        }
    }

public:
    static void parseQueryParams(HttpReq &httpReq) {
        const auto pos = httpReq.path.find('?');
        if (pos == std::string::npos) {
            return;
        }
        const std::string query = httpReq.path.substr(pos + 1);
        std::istringstream iss(query);
        std::string pair;
        while (std::getline(iss, pair, '&')) {
            if (const auto equalPos = pair.find('='); equalPos != std::string::npos) {
                std::string key = pair.substr(0, equalPos);
                std::string value = pair.substr(equalPos + 1);
                httpReq.args[trim(key)] = trim(value);
            }
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

    static std::string &trim(std::string &str) {
        const auto beginPos = str.find_first_not_of(trimList);
        if (beginPos == std::string::npos) {
            str.clear();
            return str;
        }

        const auto endPos = str.find_last_not_of(trimList);
        str = str.substr(beginPos, endPos - beginPos + 1);
        return str;
    }

    static std::string trimmed(const std::string &str) {
        std::string result = str;
        return trim(result);
    }

    static std::string getHeader(const std::string &key, const std::unordered_map<std::string, std::string> &headers) {
        return headers.contains(key) ? headers.at(key) : "";
    }

    static std::string getArg(const std::string &key, const HttpReq &httpReq) {
        return httpReq.args.contains(key) ? httpReq.args.at(key) : "";
    }

    static std::string methodToString(const HttpReq &httpReq) {
        return methodToString(httpReq.method);
    }

    static size_t getContentLength(const std::unordered_map<std::string, std::string> &headers) {
        const std::string lenStr = getHeader("Content-Length", headers);
        return lenStr.empty() ? 0 : std::stoul(lenStr);
    }

    static void addHeaderItem(std::unordered_map<std::string, std::string> &headers,
                              const std::string &key, const std::string &value) {
        headers[key] = value;
    }

    static std::optional<HttpReq> parse(Buffer &buffer, HttpReq &httpReq) {
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
                        bodyLen = getContentLength(httpReq.headers);
                        if (bodyLen == 0) {
                            state = DONE;
                        } else {
                            state = BODY;
                        }
                    } else {
                        parseHeader(line, httpReq.headers);
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

    static std::optional<HttpResp> parse(Buffer &buffer, HttpResp &httpResp) {
        ParseState state = STATUS_LINE;
        size_t bodyLen{};
        std::string line;
        while (state != DONE && buffer.getReadableSize() > 0) {
            const auto crlf = buffer.findCRLF();
            if (crlf == nullptr) break;
            const size_t lineLen = crlf - buffer.peek();
            line.assign(buffer.peek(), lineLen);
            buffer.retrieve(lineLen + lineSpilt.size());
            switch (state) {
                case STATUS_LINE:
                    if (!parseStatusLine(line, httpResp)) {
                        return std::nullopt;
                    }
                    state = HEADERS;
                    break;
                case HEADERS:
                    if (line.empty()) {
                        bodyLen = getContentLength(httpResp.headers);
                        if (bodyLen == 0) {
                            state = DONE;
                        } else {
                            state = BODY;
                        }
                    } else {
                        parseHeader(line, httpResp.headers);
                    }
                    break;
                case BODY:
                    if (buffer.getReadableSize() >= bodyLen) {
                        httpResp.body.assign(buffer.peek(), bodyLen);
                        buffer.retrieve(bodyLen);
                        state = DONE;
                    }
                    break;
            }
        }
        return state == DONE ? std::optional(httpResp) : std::nullopt;
    }

    static void print(const HttpReq &httpReq) {
        std::println("============= Request ==============");
        std::print("Method: {}\n, Path: {}\n, Version: {}\n"
                   , methodToString(httpReq), httpReq.path, httpReq.version);
        std::println("Headers:");
        for (const auto &[key, value]: httpReq.headers) {
            std::println("\t{}{}{}", key, kvSplit, value);
        }
        if (!httpReq.args.empty()) {
            std::println("Args:\n");
            for (const auto &[key, value]: httpReq.args) {
                std::println("\t{}{}{}", key, kvSplit, value);
            }
        }
        if (httpReq.body.size() > 50) {
            std::println("Body: {} ...", httpReq.body.substr(0, 50));
        } else {
            std::println("Body: {}", httpReq.body);
        }
    }

    static void print(const HttpResp &httpResp) {
        std::println("============= Response ==============");
        std::print("Version:{}\nStatus Code: {}\nStatus Text: {}\n",
                   httpResp.version, httpResp.statusCode, httpResp.statusText);
        std::println("Headers:");
        for (const auto &[key, value]: httpResp.headers) {
            std::println("\t{}{}{}", key, kvSplit, value);
        }
        if (httpResp.body.size() > 50) {
            std::println("Body: {} ...", httpResp.body.substr(0, 50));
        } else {
            std::println("Body: {}", httpResp.body);
        }
    }


    static std::string serialize(const HttpReq &httpReq) {
        // 构建完整路径（包含查询参数）
        std::string fullPath = httpReq.path;
        if (!httpReq.args.empty()) {
            fullPath += "?";
            for (const auto &[key, value]: httpReq.args) {
                fullPath.append(key).append("=").append(value).append("&");
            }
            // 移除最后的 &
            fullPath.pop_back();
        }

        // 请求行
        std::string result = std::format("{} {} {}{}",
                                         methodToString(httpReq.method),
                                         fullPath,
                                         httpReq.version,
                                         lineSpilt
        );

        // 请求头
        for (const auto &[key, value]: httpReq.headers) {
            result.append(std::format("{}{}{}{}",
                                      key, kvSplit, value, lineSpilt
            ));
        }
        if (!httpReq.headers.contains("Content-Length") && !httpReq.body.empty()) {
            result.append(std::format(
                "{}{}{}{}",
                "Content-Length", kvSplit, httpReq.body.size(), lineSpilt
            ));
        }
        // 空行及请求体
        result.append(lineSpilt);
        result.append(httpReq.body);
        return result;
    }

    static std::string serialize(const HttpResp &httpResp) {
        // 状态行
        std::string result = std::format("{} {} {}{}",
                                         httpResp.version, httpResp.statusCode,
                                         httpResp.statusText, lineSpilt
        );
        // 响应头
        for (const auto &[key, value]: httpResp.headers) {
            result.append(std::format("{}{}{}{}",
                                      key, kvSplit, value, lineSpilt
            ));
        }
        if (!httpResp.headers.contains("Content-Length") && !httpResp.body.empty()) {
            result.append(std::format(
                "{}{}{}{}",
                "Content-Length", kvSplit, httpResp.body.size(), lineSpilt
            ));
        }
        // 空行及响应体
        result.append(lineSpilt);
        result.append(httpResp.body);
        return result;
    }
};

inline std::string HttpUtils::lineSpilt = "\r\n";
inline std::string HttpUtils::kvSplit = ": ";
inline std::string HttpUtils::trimList = " \r\t\n";


class HttpRequest {
public:
    std::string getHeader(const std::string &key) const {
        return HttpUtils::getHeader(key, _msg.headers);
    }

    std::string getArg(const std::string &key) const {
        return HttpUtils::getArg(key, _msg);
    }

    std::string methodToString() const {
        return HttpUtils::methodToString(_msg.method);
    }

    void print() const {
        return HttpUtils::print(_msg);
    }

    std::optional<HttpReq> parse(Buffer &buffer) {
        return HttpUtils::parse(buffer, _msg);
    }

    void addHeaderItem(const std::string &key, const std::string &value) {
        HttpUtils::addHeaderItem(_msg.headers, key, value);
    }

    std::string serialize() const {
        return HttpUtils::serialize(_msg);
    }

// private:
    // 这里 message 取 “报文” 的意思
    HttpReq _msg{};
};

class HttpResponse {
public:
    std::string getHeader(const std::string &key) const {
        return HttpUtils::getHeader(key, _msg.headers);
    }

    std::optional<HttpResp> parse(Buffer &buffer) {
        return HttpUtils::parse(buffer, _msg);
    }

    void print() const {
        return HttpUtils::print(_msg);
    }

    std::string serialize() const {
        return HttpUtils::serialize(_msg);
    }

// private:
    HttpResp _msg{};
};
