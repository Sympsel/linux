#include <memory>

#include "Client.hpp"

int main() {
    const auto appClient = std::make_unique<Client>();
    appClient->setServer("127.0.0.1", 8123);
    appClient->run();
    return 0;
}
