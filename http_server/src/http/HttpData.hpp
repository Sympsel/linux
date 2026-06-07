#pragma once

#include <sstream>
#include <string>
#include <unordered_map>

enum class HttpMethod {
    GET, POST, PUT, DELETE, HEAD, UNKNOWN
};

namespace http_v {
    inline constexpr std::string line_spilt = "\r\n";
    inline constexpr std::string kv_split = ": ";
    inline constexpr std::string http_version = "HTTP/1.1";
    inline constexpr std::string space = " ";
}


struct HttpRequest {
    HttpMethod method = HttpMethod::UNKNOWN;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string getHeader(const std::string &key) const {
        return headers.contains(key) ? headers.at(key) : "";
    }

    void setHeader(const std::string &key, const std::string &value) {
        headers[key] = value;
    }

    void print(const std::string &args = "") const {
        std::cout << "============= Request ==============" << std::endl;
        std::cout << "Method: " << methodToString() << std::endl;
        std::cout << "Path: " << path << std::endl;
        std::cout << "Version: " << version << std::endl;
        std::cout << "Headers:" << std::endl;
        for (const auto &[key, value]: headers) {
            std::cout << "\t" << key << ": " << value << std::endl;
        }
        if (!args.empty()) {
            std::cout << "Args: " << args << std::endl;
        }
        std::cout << "Body: " << body << std::endl;
    }

    std::string methodToString() const {
        return methodToString(method);
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
};

struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string getHeader(const std::string &key) const {
        return headers.contains(key) ? headers.at(key) : "";
    }

    void setHeader(const std::string &key, const std::string &value) {
        headers[key] = value;
    }

    std::string serialize() const {
        using namespace http_v;
        std::string result;
        // 状态行
        result += http_version + space
                + std::to_string(status_code) + space
                + status_text + line_spilt;

        // 响应头
        for (const auto &[key, value]: headers) {
            std::stringstream ss;
            ss << key << kv_split << value << line_spilt;
            result += ss.str();
        }
        if (!headers.contains("Content-Length") && !body.empty()) {
            result += "Content-Length";
            result += kv_split + std::to_string(body.size()) + line_spilt;
        }
        // 空行
        result += line_spilt;
        // 响应体
        result += body;
        return result;
    }

    void print() const {
        std::cout << "============= Response ==============" << std::endl;
        std::cout << "Status Code: " << status_code << std::endl;
        std::cout << "Status Text: " << status_text << std::endl;
        std::cout << "Headers:" << std::endl;
        for (const auto &[key, value]: headers) {
            std::cout << "\t" << key << ": " << value << std::endl;
        }
        std::cout << "Body: " << body << std::endl;
    }
};
