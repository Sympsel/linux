#pragma once

#include <filesystem>

#include "Log.hpp"


namespace fs = std::filesystem;

class FileReader {
public:
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
                return {};
            }

            // 检查是否是普通文件
            if (!fs::is_regular_file(filepath)) {
                LOG_WARN() << "Not a regular file: " << filepath;
                return {};
            }

            // 检查文件大小（限制最大 10MB）
            const auto file_size = fs::file_size(filepath);
            constexpr auto max_size = 10 * 1024 * 1024; // 10MB
            if (file_size > max_size) {
                LOG_WARN() << "File too large: " << filepath << " (" << file_size << " bytes)";
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
};