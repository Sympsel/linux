#pragma once
#include <string>
#include <unordered_map>

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