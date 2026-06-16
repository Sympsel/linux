#include "Reactor.hpp"
#include "Listener.h"
#include "Connection.hpp"

constexpr uint16_t port = 8080;

int main() {
    const auto connection = std::make_shared<Listener>(port);
    connection->setEventItem(IN | ET);
    const auto reactorServer = std::make_unique<Reactor>();
    reactorServer->addConnection(connection);
    reactorServer->dispatcher();
    return 0;
}