#pragma once

#include <filesystem>

#include "../http/HttpData.hpp"
#include "../utils/FileReader.hpp"
#include "../utils/Log.hpp"
#include "PathManager.hpp"


namespace fs = std::filesystem;

class StaticFileHandle {
public:
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

        // 测试重定向功能
        if (relative_path == "deprecated_page.html") {
            redirect(request, response, path_mgr.getWebRoot() / "deprecated_page.html");
            return;
        }

        LOG_INFO() << "Serving static file: " << relative_path;

        // 解析安全路径
        const fs::path full_path = path_mgr.resolveSafePath(relative_path);
        if (full_path.empty()) {
            LOG_WARN() << "Access denied or invalid path: " << request.path;
            handleNotFound(response);
            return;
        }

        // 读取文件
        std::string content = FileReader::readFile(full_path);
        if (content.empty()) {
            LOG_WARN() << "File not found: " << request.path;
            handleNotFound(response);
        } else {
            // 设置响应
            response.status_code = 200;
            response.status_text = "OK";
            response.body = std::move(content);
            response.setHeader("Content-Type", PathManager::getMimeType(full_path));

            LOG_DEBUG() << "Successfully served: " << request.path
                    << " (" << response.body.size() << " bytes)";
        }
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
        std::string content = FileReader::readFile(full_path);
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


    static void redirect(const HttpRequest &request, HttpResponse &response, const fs::path &to) {
        response.status_code = 302;
        response.status_text = "Found";

        response.body = FileReader::readFile(to);
        response.setHeader("Content-Type", PathManager::getMimeType(to));
        response.setHeader("Location", to.string());
        response.setHeader("Content-Length", std::to_string(response.body.size()));
    }

    static void handleNotFound(HttpResponse &response) {
        const std::string full_path = PathManager::getInstance().resolveSafePath("404.html");
        response.status_code = 404;
        response.setHeader("Content-Type", "text/html; charset=utf-8");
        response.status_text = "Not Found";
        response.body = FileReader::readFile(full_path);
    }

    static void handleNotFound(const HttpRequest &request, HttpResponse &response) {
        handleNotFound(response);
        // 其他业务...
    }
};