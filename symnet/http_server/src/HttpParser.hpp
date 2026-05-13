#pragma once
#include <sstream>
#include <string>

#include "DataType.hpp"

class HttpParser {
public:
    static bool ParseRequest(const std::string &raw_req, HttpRequest& out) {
        std::string_view sv{raw_req};

        // 解析请求行: method path version\r\n
        auto line_end = sv.find("\r\n");
        if (line_end == std::string::npos) return false;
        std::string request_line{sv.substr(0, line_end)};
        if (std::istringstream iss{request_line}; !(iss >> out.method >> out.path >> out.version)) return false;

        // 解析头部,找到\r\n\r\n
        auto head_start = line_end + 2;
        auto body_start = raw_req.find("\r\n\r\n", head_start);
        if (body_start == std::string::npos) {
            // 没有body, 但是需要保证头部完整
            body_start = raw_req.find("\r\n", head_start);
            if (body_start == std::string::npos ||
                body_start != raw_req.size() - 2) {
                return false; // 头部不完整
            }
            body_start = head_start;
        } else {
            body_start += 4;
        }

        // 解析头部字段
        std::string head{raw_req.substr(head_start, body_start - head_start - 4)};
        std::istringstream head_iss{head};
        std::string header_line;
        while (std::getline(head_iss, header_line)) {
            // 去除 \r
            if (!header_line.empty() && header_line.back() == '\r') {
                header_line.pop_back();
            }

            if (header_line.empty()) continue;
            auto colon_pos = header_line.find(':');
            // 如果没找到冒号，则跳过该项
            if (colon_pos == std::string::npos) continue;
            std::string key = header_line.substr(0, colon_pos);
            std::string value = header_line.substr(colon_pos + 1);
            Trim(key);
            Trim(value);
            ToLower(key);
            out.headers[key] = value;
        }
        if (body_start < raw_req.size()) {
            out.body = raw_req.substr(body_start);
        }
        return true;
    }

    // 构建标准 HTTP 响应
    static std::string BuildResponse(const HttpResponse& resp) {
        std::ostringstream oss;

        // 状态行
        oss << "HTTP/1.1 " << resp.status_code << " " << resp.status_text << "\r\n";

        // 头部
        for (const auto& [key, value] : resp.headers) {
            oss << key << ": " << value << "\r\n";
        }

        // 如果没有 Content-Length 头部字段，则添加
        if (!resp.headers.contains("content-length") &&
            !resp.headers.contains("Contains-Length")) {
            oss << "Content-Length: " << resp.body.size() << "\r\n";
        }
        oss << "\r\n";

        // Body
        oss << resp.body;
        return oss.str();
    }
private:
    // 删除字符串首尾的空格
    static void Trim(std::string& str) {
        auto start = str.begin();
        auto end = str.end();
        while (start != end && std::isspace(*start)) ++start;
        while (start != end && std::isspace(*end)) --end;
        str = std::string(start, end);
    }

    // 字符串转小写
    static void ToLower(std::string& str) {
        for (auto& c : str) {
            c = static_cast<char>(std::tolower(c));
        }
    }
};
