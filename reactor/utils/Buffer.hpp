#pragma once

#include <string>
#include <cstring>
#include <vector>

class Buffer {
public:
    explicit Buffer(const size_t initialSize = 1024)
        : _buffer(initialSize), _readPos(), _writePos() {
    }

    void append(const char *data, const size_t len) {
        ensureWritableSizeOK(len);
        std::memcpy(&_buffer[_writePos], data, len);
        _writePos += len;
    }

    void append(const std::string &data) {
        append(data.c_str(), data.size());
    }

    [[nodiscard]] const char *peek() const {
        return &_buffer[_readPos];
    }

    /**
     * @brief 取出已使用的长度为 len 的数据并丢弃，若数据不足，有多少取多少
     */
    void retrieve(const size_t len) {
        if (len < getReadableSize()) {
            _readPos += len;
        } else {
            retrieveAll();
        }
    }

    void retrieveAll() {
        _readPos = _writePos = 0;
    }

    void ensureWritableSizeOK(const size_t len) {
        if (getWritableSize() < len) {
            makeSpace(len);
        }
    }

    [[nodiscard]] size_t getWritableSize() const {
        return _buffer.size() - _writePos;
    }

    [[nodiscard]] size_t getReadableSize() const {
        return _writePos - _readPos;
    }

    [[nodiscard]] bool empty() const {
        return _readPos == _writePos;
    }

    [[nodiscard]] const char* findCRLF() const {
        const std::string_view readView{peek(), getReadableSize()};
        const auto pos = readView.find("\r\n");
        return pos == std::string::npos ? nullptr : readView.data() + pos;
    }

private:
    /**
     * @brief 如果回收已读缓冲区仍然无法满足需求，则进行扩容, 否则仅回收已读缓冲区
     * @param len 要写入的数据长度
     */
    void makeSpace(const size_t len) {
        if (_readPos + getWritableSize() < len) {
            _buffer.resize(_writePos + len);
        } else {
            const size_t readableSize = getReadableSize();
            std::memmove(&_buffer[0], &_buffer[_readPos], readableSize);
            _readPos = 0;
            _writePos = readableSize;
        }
    }

    std::vector<char> _buffer;
    size_t _readPos;
    size_t _writePos;
};
