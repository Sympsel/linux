#pragma once

#include <filesystem>
#include <unordered_map>

#include "Log.hpp"

namespace fs = std::filesystem;

class PathManager {
public:
    static PathManager &getInstance() {
        static PathManager instance;
        return instance;
    }

    // 初始化路径（在 main 中调用）
    void initialize(const fs::path &executable_path) {
        _exe_dir = executable_path.parent_path();

        // 推断项目根目录
        if (isBuildDirectory(_exe_dir)) {
            _project_root = _exe_dir.parent_path();
        } else {
            _project_root = _exe_dir;
        }

        // 设置 web 根目录
        _web_root = _project_root / "public";

        // 验证目录是否存在
        if (!fs::exists(_web_root)) {
            LOG_WARN() << "Web root directory does not exist: " << _web_root;
            fs::create_directories(_web_root);
            LOG_INFO() << "Created web root directory: " << _web_root;
        }

        LOG_INFO() << "Executable dir: " << _exe_dir;
        LOG_INFO() << "Project root: " << _project_root;
        LOG_INFO() << "Web root: " << _web_root;
    }

    // 获取 Web 根目录
    [[nodiscard]] const fs::path &getWebRoot() const {
        return _web_root;
    }

    /**
     *
     * @param relative_path  相对路径
     * @return 安全的文件路径, 确保所有文件都在 web_root 下
     */
    [[nodiscard]] fs::path resolveSafePath(const std::string &relative_path) const {
        fs::path full_path = _web_root / relative_path;

        try {
            // 仅规范化路径, 不保证路径是否存在
            full_path = fs::weakly_canonical(full_path);
        } catch (const fs::filesystem_error &e) {
            LOG_ERROR() << "Path resolution error: " << e.what();
            return {};
        }

        // 确保路径在 web_root 内
        if (!isPathWithinRoot(full_path, _web_root)) {
            LOG_WARN() << "Security violation - path traversal attempt: " << relative_path;
            return {};
        }

        return full_path;
    }

    /**
     * @brief 是根据文件扩展名判断文件的媒体类型,
     * 以便在 HTTP 响应中设置正确的 Content-Type 头,
     * 保证浏览器对对应文件的渲染效果或二进制文件的下载
     * @param filepath 文件路径
     * @return 文件媒体类型
     */
    static std::string getMimeType(const fs::path &filepath) {
        // 获取文件扩展名
        const std::string ext = filepath.extension().string();

        static const std::unordered_map<std::string, std::string> mime_types = {
            {".html", "text/html; charset=utf-8"},
            {".htm", "text/html; charset=utf-8"},
            {".css", "text/css"},
            {".js", "application/javascript"},
            {".json", "application/json"},
            {".png", "image/png"},
            {".jpg", "image/jpeg"},
            {".jpeg", "image/jpeg"},
            {".gif", "image/gif"},
            {".svg", "image/svg+xml"},
            {".ico", "image/x-icon"},
            {".txt", "text/plain"},
            {".xml", "application/xml"},
            {".pdf", "application/pdf"},
            {".woff", "font/woff"},
            {".woff2", "font/woff2"},
            {".ttf", "font/ttf"},
        };
        // 查找扩展名对应的 MIME 类型，找不到则返回二进制流
        return mime_types.contains(ext) ? mime_types.at(ext) : "application/octet-stream";
    }

private:
    PathManager() = default;

    // 程序执行目录
    fs::path _exe_dir;
    // 项目根目录
    fs::path _project_root;
    // Web 根目录
    fs::path _web_root;

    // 检查是否是构建目录
    static bool isBuildDirectory(const fs::path &dir) {
        const std::string dir_name = dir.filename().string();
        return dir_name == "cmake-build-debug" ||
               dir_name == "cmake-build-release" ||
               dir_name == "build" ||
               dir_name == "bin";
    }

    /**
     *
     * @param path 要查询的路径
     * @param root 跟目录
     * @return true if path is within root, false otherwise
     */
    static bool isPathWithinRoot(const fs::path &path, const fs::path &root) {
        try {
            const auto rel = fs::relative(path, root);
            // 如果第一个字符是 '.'，则表示路径查询非法
            return !rel.empty() && rel.native()[0] != '.';
        } catch (...) {
            return false;
        }
    }
};
