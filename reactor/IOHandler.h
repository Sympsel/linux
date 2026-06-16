#pragma once

#include <functional>

#include "Log.hpp"
#include "Connection.hpp"

class Reactor;

using messageHandler = std::function<std::string(std::string &)>;

class IOHandler : public Connection {
private:
    static std::string handleRequest(std::string& request);
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
    std::string _inBuffer;
    std::string _outBuffer;
    int _clientSockFd{-1};
    // messageHandler _onMsg;
};