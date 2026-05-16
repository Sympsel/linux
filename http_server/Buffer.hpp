#pragma once

#include <cstring>
#include <string>
#include <vector>

class Buffer {
public:
    Buffer(const size_t initial_size = 1024)
        : _buffer(initial_size), _read_index(), _write_index() {
    }

    void append(const char* data, const size_t len) {
        ensureWritableBytes(len);
        std::memcpy(&_buffer[_write_index], data, len);
        _write_index += len;
    }

    void append(const std::string& data) {
        append(data.c_str(), data.size());
    }

    /**
     *
     * @return nullptr if not found
     */
    [[nodiscard]] const char* findCRLF() const {
        auto view = std::string_view(peek(), getReadableBytes());
        const std::size_t pos = view.find("\r\n");
        return pos == std::string::npos ? nullptr : view.data() + pos;
    }

    // 仅读取不移动读指针
    [[nodiscard]] const char* peek() const {
        return &_buffer[_read_index];
    }

    /**
     *
     * @brief 向后移动读指针, 移动长度为len和可读长度的较小值
     * @param len 要读取的长度
     */
    void retrieve(const size_t len) {
        if (len < getReadableBytes()) {
            _read_index += len;
        } else {
            // 不够读就能读多少读多少
            retrieveAll();
        }
    }

    void retrieveAll() {
        _read_index = _write_index = 0;
    }

    [[nodiscard]] size_t getWritableBytes() const {
        return _buffer.size() - _write_index;
    }

    [[nodiscard]] size_t getReadableBytes() const {
        return _write_index - _read_index;
    }

    void ensureWritableBytes(const size_t len) {
        if (getWritableBytes() < len) {
            makeSpace(len);
        }
    }

private:
    void makeSpace(const size_t len) {
        // 如果回收已读缓冲区仍然无法满足需求，则进行扩容, 否则仅回收已读缓冲区
        if (getWritableBytes() + _read_index < len) {
            _buffer.resize(_write_index + len);
        } else {
            // 将数据向前移动
            const size_t readable = getReadableBytes();
            std::memmove(&_buffer[0], &_buffer[_read_index], readable);
            _read_index = 0;
            _write_index = readable;
        }
    }

    std::vector<char> _buffer;
    size_t _read_index;
    size_t _write_index;
};
