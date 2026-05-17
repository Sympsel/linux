#pragma once

#include <fcntl.h>

#include <functional>
#include <memory>

#include "../utils/Buffer.hpp"
#include "../utils/Log.hpp"

// 继承 std::enable_shared_from_this<TcpConnection>
// 是为了让类能够安全地从成员函数内部获取指向自身的 shared_ptr
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    using Ptr = std::shared_ptr<TcpConnection>;
    using MessageCallback = std::function<void(const Ptr &, Buffer &)>;
    using CloseCallback = std::function<void(const Ptr &)>;

private:
    /**
     * @brief 保留原有的所有标志, 添加非阻塞标志
     */
    void setNonBlocking() const {
        // 获取原有的标志
        const int flags = ::fcntl(_sockfd, F_GETFL, 0);
        // 追加非阻塞标志
        ::fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);
    }

public:
    TcpConnection(const int sockfd)
        : _sockfd(sockfd),
          _input_buf(std::make_unique<Buffer>()),
          _output_buf(std::make_unique<Buffer>()) {
        setNonBlocking();
    }

    ~TcpConnection() {
        if (_sockfd >= 0) {
            ::close(_sockfd);
        }
    }

    // 处理可读事件
    void handleRead() {
        char buf[65536];
        if (const ssize_t n = ::read(_sockfd, buf, sizeof buf);
            n > 0) {
            _input_buf->append(buf, n);
            if (_msg_cb) {
                _msg_cb(shared_from_this(), *_input_buf);
            } else {
                LOG_ERROR() << "尚未注册消息处理行为...";
            }
        } else if (n == 0) {
            // 连接关闭
            if (_close_cb) {
                _close_cb(shared_from_this());
            } else {
                LOG_WARN() << "尚未注册连接关闭行为...";
            }
        } else {
            LOG_ERROR() << "read error";
            if (_close_cb) {
                _close_cb(shared_from_this());
            } else {
                LOG_WARN() << "尚未注册连接关闭行为...";
            }
        }
    }

    // 发送数据
    void send(const std::string &data) const {
        if (const ssize_t n = ::write(_sockfd, data.c_str(), data.size());
            // 如果发送的数据小于实际发送的数据，则将剩余数据保存在输出缓冲区中
            n < data.size()) {
            _output_buf->append(data.c_str() + n, data.size() - n);
            // todo 配合可写时间来继续发送剩余数据
            LOG_WARN() << "数据未完全发送...";
        }
    }

    [[nodiscard]] int getSockfd() const {
        return _sockfd;
    }

    void setMessageCallback(MessageCallback cb) {
        _msg_cb = std::move(cb);
    }

    void setCloseCallback(CloseCallback cb) {
        _close_cb = std::move(cb);
    }

private:
    int _sockfd;
    std::unique_ptr<Buffer> _input_buf;
    std::unique_ptr<Buffer> _output_buf;
    MessageCallback _msg_cb;
    CloseCallback _close_cb;
};
