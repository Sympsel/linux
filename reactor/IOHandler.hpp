#pragma once

#include "Log.hpp"
#include "Connection.hpp"

class IOHandler : public Connection {
public:
    explicit IOHandler() : _sockfd() {
    }

    void receiver() override {
    }

    void sender() override {
    }

    void expecter() override {
    }

    int getSockFd() override {
        return _sockfd;
    }

    ~IOHandler() override = default;

private:
    int _sockfd;
};
