#include <memory>
#include <print>
#include "SelectServer.hpp"

constexpr uint16_t gPort = 8080;

int main() {
    auto select_server = std::make_unique<SelectServer>(gPort);
    select_server->start();
    return 0;
}