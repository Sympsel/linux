#pragma once

#include <functional>

#include "Buffer.hpp"
#include "Log.hpp"
#include "Connection.hpp"

class Reactor;

using messageHandler = std::function<std::string(std::string &)>;

class IOHandler : public Connection {
private:
    static std::string handleRequest(Buffer &request);

public:
    explicit IOHandler(const int fd) : _clientSockFd(fd) {
    }

    void receiver() override;


    void sender() override;

    void expecter() override;

    int getSockFd() override {
        return _clientSockFd;
    }

    // void registerMessageHandler(const messageHandler& onMsg) {
    // _onMsg = onMsg;
    // }

    ~IOHandler() override = default;

private:
    Buffer _inBuffer;
    Buffer _outBuffer;
    int _clientSockFd{-1};
    // messageHandler _onMsg;
};
