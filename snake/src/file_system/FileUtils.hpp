#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

class FileUtils {
public:
    static bool ReadLine(const fs::path &path, std::string &line_to_fill) {
        static std::ifstream file{path};
        if (!file) {
            throw std::runtime_error("File not found: " + path.string());
        }
        return static_cast<bool>(std::getline(file, line_to_fill));
    }

    static std::vector<std::byte> ReadBinary(const fs::path &path) {
        std::ifstream file{path, std::ios::binary};
        if (!file) {
            throw std::runtime_error("File not found: " + path.string());
        }
        file.seekg(0, std::ios::end);
        const auto size{file.tellg()};
        file.seekg(0, std::ios::beg);
        std::vector<std::byte> buffer(size);
        file.read(reinterpret_cast<std::istream::char_type *>(buffer.data()), size);
        return buffer;
    }

    // static
};
