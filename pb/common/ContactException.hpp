#pragma once

#include <string>

class ContactException : public std::exception {
public:
    explicit ContactException(std::string message = "程序出错")
        : _message(std::move(message)) {
    }

    [[nodiscard]] const char* what() const noexcept override {
        return _message.c_str();
    }

private:
    std::string _message;
};
