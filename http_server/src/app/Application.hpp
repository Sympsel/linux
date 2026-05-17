#pragma once

#include "../http/HttpServer.hpp"
#include "PathManager.hpp"

class Application {
public:
    static void handleRequest(const HttpRequest &request, HttpResponse &response) {
        LOG_INFO() << "Request: " <<
                methodToString(request.method) << " "
                << request.path << " "
                << request.version;

        const std::string &path = request.path;
        // 优先匹配 API 路由
        if (path == "/api/users") {
            handleGetUsers(request, response);
        } else if (path == "/api/echo") {
            handleEcho(request, response);
        } else {
            // 跟路径和静态文件
            serveStaticFile(request, response);
        }
    }

private:
    static std::string methodToString(const HttpMethod &method) {
        switch (method) {
            case HttpMethod::GET: return "GET";
            case HttpMethod::POST: return "POST";
            case HttpMethod::PUT: return "PUT";
            case HttpMethod::DELETE: return "DELETE";
            case HttpMethod::HEAD: return "HEAD";
            default: return "UNKNOWN";
        }
    }

    /**
     *
     * @param filepath 文件路径
     * @return 文件内容
     */
    static std::string readFile(const fs::path &filepath) {
        try {
            // 检查文件是否存在
            if (!fs::exists(filepath)) {
                LOG_WARN() << "File not found: " << filepath;
                return "";
            }

            // 检查是否是普通文件
            if (!fs::is_regular_file(filepath)) {
                LOG_WARN() << "Not a regular file: " << filepath;
                return "";
            }

            // 检查文件大小（限制最大 10MB）
            const auto file_size = fs::file_size(filepath);
            constexpr auto max_size = 10 * 1024 * 1024; // 10MB
            if (file_size > max_size) {
                LOG_WARN() << "File too large: " << filepath
                        << " (" << file_size << " bytes)";
                return {};
            }

            // 以二进制模式读取
            std::ifstream file(filepath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                LOG_ERROR() << "Failed to open file: " << filepath;
                return {};
            }

            // 预分配内存
            std::string content(file.tellg(), '\0');
            file.seekg(0, std::ios::beg);
            file.read(content.data(), static_cast<std::streamsize>(content.size()));

            if (!file) {
                LOG_ERROR() << "Failed to read file: " << filepath;
                return {};
            }

            LOG_DEBUG() << "Successfully read file: " << filepath
                    << " (" << content.size() << " bytes)";

            return content;
        } catch (const fs::filesystem_error &e) {
            LOG_ERROR() << "Filesystem error: " << e.what();
            return {};
        } catch (const std::exception &e) {
            LOG_ERROR() << "Error reading file: " << e.what();
            return {};
        }
    }

    /**
     *
     * @brief 提供静态文件服务
     */
    static void serveStaticFile(const HttpRequest &request, HttpResponse &response) {
        const auto &path_mgr = PathManager::getInstance();

        // 获取请求路径
        std::string path = request.path;

        // 如果是根路径或目录，默认返回 index.html
        if (path == "/" || path.empty()) {
            path = "/index.html";
        }

        // 移除前导 /，转换为相对路径
        const std::string relative_path = path.substr(1);

        LOG_INFO() << "Serving static file: " << relative_path;

        // 解析安全路径
        const fs::path full_path = path_mgr.resolveSafePath(relative_path);
        if (full_path.empty()) {
            LOG_WARN() << "Access denied or invalid path: " << request.path;
            handleNotFound(response);
            return;
        }

        // 读取文件
        std::string content = readFile(full_path);
        if (content.empty()) {
            LOG_WARN() << "File not found: " << request.path;
            handleNotFound(response);
            return;
        }

        // 设置响应
        response.status_code = 200;
        response.status_text = "OK";
        response.body = std::move(content);
        response.setHeader("Content-Type", PathManager::getMimeType(full_path));

        LOG_DEBUG() << "Successfully served: " << request.path
                << " (" << response.body.size() << " bytes)";
    }

    // 提供静态文件服务
    [[deprecated]] static bool serveStaticFile(const std::string &path, HttpResponse &response) {
        const auto &path_mgr = PathManager::getInstance();

        // 解析安全路径
        const fs::path full_path = path_mgr.resolveSafePath(path);
        if (full_path.empty()) {
            return false;
        }

        // 读取文件
        std::string content = readFile(full_path);
        if (content.empty()) {
            return false;
        }

        // 设置响应
        response.status_code = 200;
        response.status_text = "OK";
        response.body = std::move(content);
        response.setHeader("Content-Type", PathManager::getMimeType(full_path));

        return true;
    }

    static void handleHome(const HttpRequest &request, HttpResponse &response) {
        // 直接调用通用静态文件服务
        serveStaticFile(request, response);
    }

    static void handleGetUsers(const HttpRequest &request, HttpResponse &response) {
        response.status_code = 200;
        response.status_text = "OK";
        response.body = R"({"users": ["Alice", "Bob", "Charlie"]})";
        response.setHeader("Content-Type", "application/json");
    }

    static void handleEcho(const HttpRequest &request, HttpResponse &response) {
        response.status_code = 200;
        response.status_text = "OK";
        response.body = "Echo: " + request.body;
        response.setHeader("Content-Type", "text/plain");
    }

    static void handleNotFound(HttpResponse &response) {
        response.status_code = 404;
        response.status_text = "Not Found";
        response.body = R"(
<html>
<head><title>404 Not Found</title></head>
<body>
<h1>404 Not Found</h1>
<p>The requested resource was not found.</p>
</body>
</html>
)";
        response.setHeader("Content-Type", "text/html; charset=utf-8");
    }

    static void handleNotFound(const HttpRequest &request, HttpResponse &response) {
        response.status_code = 404;
        response.status_text = "Not Found";
        response.body = R"(
<html>
<head><title>404 Not Found</title></head>
<body>
<h1>404 Not Found</h1>
<p>The requested resource was not found.</p>
</body>
</html>
)";
        response.setHeader("Content-Type", "text/html; charset=utf-8");
    }
};
