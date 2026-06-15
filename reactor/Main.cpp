#include "TcpServer.hpp"
#include "Listener.hpp"
#include "Connection.hpp"

constexpr uint16_t port = 8080;

int main() {
    const auto connection = std::make_shared<Listener>(port);

    const auto connectionManager = std::make_unique<TcpServer>();
    connectionManager->add(connection);

    connectionManager->dispatcher();
    return 0;
}