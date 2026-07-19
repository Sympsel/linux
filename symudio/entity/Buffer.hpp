#pragma once

#include <string>
#include <cstring>
#include <utility>
#include <vector>

class Buffer {
public:
    explicit Buffer(const size_t initialSize = 1024)
        : _buffer(initialSize), _readPos(), _writePos() {
    }

    Buffer(const Buffer &other) = default;

    Buffer(Buffer &&other) noexcept
        : _buffer(std::move(other._buffer)),
          _readPos(std::exchange(other._readPos, 0)),
          _writePos(std::exchange(other._writePos, 0)) {
    }

    Buffer &operator=(const Buffer &other) = default;

    Buffer &operator=(Buffer &&other) noexcept {
        if (this != &other) {
            _buffer = std::move(other._buffer);
            _readPos = std::exchange(other._readPos, 0);
            _writePos = std::exchange(other._writePos, 0);
        }
        return *this;
    }

    void write(const char *data, const size_t len) {
        ensureWritableSizeOK(len);
        std::memcpy(&_buffer[_writePos], data, len);
        _writePos += len;
    }

    void write(const std::string &data) {
        write(data.c_str(), data.size());
    }

    void write(const Buffer &buffer) {
        write(buffer.peek(), buffer.getReadableSize());
    }

    [[nodiscard]] std::string read(size_t len) {
        if (getReadableSize() < len) {
            len = getReadableSize();
        }
        std::string ret{&_buffer[_readPos], len};
        retrieve(len);
        return ret;
    }

    [[nodiscard]] std::string readOnlyPeek(size_t len) const {
        if (getReadableSize() < len) {
            len = getReadableSize();
        }
        return std::string{&_buffer[_readPos], len};
    }

    std::string readLine(const std::string &lineSplit = "\n") {
        std::string line{peekLine(lineSplit)};
        retrieve(line.size() + lineSplit.size());
        return line;
    }

    std::string peekLine(const std::string &lineSplit = "\n") {
        const std::string_view readView{peek(), getReadableSize()};
        const auto pos = readView.find(lineSplit);
        if (pos == std::string::npos) {
            return {};
        }
        std::string line{readView.data(), pos};
        retrieve(line.size() + lineSplit.size());
        return line;
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

    [[deprecated]] [[nodiscard]] const char *findCRLF() const {
        const std::string_view readView{peek(), getReadableSize()};
        const auto pos = readView.find("\r\n");
        return pos == std::string::npos ? nullptr : readView.data() + pos;
    }

    [[deprecated]] [[nodiscard]] const char *findLF() const {
        const std::string_view readView{peek(), getReadableSize()};
        const auto pos = readView.find('\n');
        return pos == std::string::npos ? nullptr : readView.data() + pos;
    }

    Buffer &swap(Buffer &other) noexcept {
        other = std::exchange(*this, other);
        return *this;
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
