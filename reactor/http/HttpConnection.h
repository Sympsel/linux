#pragma once

#include <memory>
#include <functional>

#include "Connection.hpp"
#include "HttpProtocol.hpp"

class Reactor;

using HttpRequestHandler = std::function<void(const HttpRequest &, HttpResponse &)>;

class HttpConnection : public Connection {
public:
    explicit HttpConnection(const int fd)
        : _sockFd(fd), _httpRequest(std::make_unique<HttpRequest>()),
          _httpResponse(std::make_unique<HttpResponse>()) {
    }

    void receiver() override;

    void sender() override;

    void expecter() override;

    int getSockFd() override {
        return _sockFd;
    }

    void setRequestHandler(HttpRequestHandler handler) {
        _requestHandler = std::move(handler);
    }

    ~HttpConnection() override = default;

private:
    bool parseRequest();

    void handleRequest() const;

    void sendResponse();

    void handleClose();

private:
    int _sockFd{-1};
    std::unique_ptr<HttpRequest> _httpRequest;
    std::unique_ptr<HttpResponse> _httpResponse;
    HttpRequestHandler _requestHandler;
};
