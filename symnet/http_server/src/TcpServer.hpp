#pragma once

#include <cstring>
#include <csignal>
#include <memory>

#include "../../utils/module/SymNet.h"

template<class TaskType>
class TcpServer {
private:
    void HandleIO(const std::shared_ptr<ITcpSocket> &sockfd, const InetAddr &client_addr) {
        std::string inbuffer;

        while (true) {
            std::string chunk;

            if (const ssize_t n = sockfd->Recv(chunk, 4096); n < 0) {
                LOG_WARN() << "recv error: " << strerror(errno);
                break;
            } else if (n == 0) {
                LOG_INFO() << "client closed";
                break;
            }

            // chunk 已经被 Recv 填充，直接追加到 inbuffer
            inbuffer += chunk;

            // 检查是否收到完整的 HTTP 头部（\r\n\r\n）
            const auto header_end = inbuffer.find("\r\n\r\n");
            if (header_end == std::string::npos) {
                continue; // 继续读取
            }

            // 解析 Content-Length 确定是否需要继续读 body
            size_t content_length = 0;
            if (const auto cl_pos = inbuffer.find("Content-Length: ");
                cl_pos != std::string::npos && cl_pos < header_end) {
                const auto cl_end = inbuffer.find("\r\n", cl_pos);
                std::string cl_str = inbuffer.substr(cl_pos + 16, cl_end - cl_pos - 16);
                content_length = std::stoul(cl_str);
            }

            // 检查是否收到完整的 body
            if (const size_t total_expected = header_end + 4 + content_length; inbuffer.size() < total_expected) {
                continue; // 继续读取 body
            }

            // 处理请求
            std::string out_buffer;
            if (_task == nullptr) {
                out_buffer = "HTTP/1.1 500 Internal Server Error\r\n"
                        "Content-Length: 35\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "server error, it's not your fault";
            } else {
                out_buffer = _task(inbuffer);
            }

            // 确保发送完整
            size_t sent = 0;
            while (sent < out_buffer.size()) {
                const ssize_t ret = sockfd->Send(out_buffer.substr(sent));
                if (ret < 0) {
                    LOG_WARN() << "send error: " << strerror(errno);
                    break;
                }
                sent += ret;
            }

            // HTTP/1.1 默认 keep-alive，但简单处理：关闭连接
            break;
        }
    }

public:
    explicit TcpServer(const in_port_t port)
        : _port(port),
          _listen_sockfd(std::make_unique<TcpSocket>()),
          _running(false),
          _task(nullptr) {
        _listen_sockfd->BuildServerSocketMethod(_port);
    }

    void Run(const TaskType &task) {
        _task = task;
        _running = true;
        // ReSharper disable once CppDFAConstantConditions
        while (_running) {
            signal(SIGCHLD, SIG_IGN);

            InetAddr client_addr;
            auto sockfd = _listen_sockfd->Acceptor(client_addr);
            if (sockfd == nullptr) {
                LOG_ERROR() << "accept failed, continuing...";
                continue;
            }

            LOG_DEBUG() << "get a new link: [" << client_addr.GetIp() << ":" << client_addr.
                    GetPort() << ", " << "conn_sockfd: " << sockfd->GetSockfd() << "]";

            const pid_t pid = fork();
            if (pid < 0) {
                LOG_FATAL() << "fork error: " << strerror(errno);
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                _listen_sockfd->Close();
                HandleIO(sockfd, client_addr);
                sockfd->Close();
                exit(EXIT_SUCCESS);
            }
            sockfd->Close();
        }
    }

private:
    int _port;
    std::unique_ptr<ITcpSocket> _listen_sockfd;
    bool _running;

    TaskType _task;
};
