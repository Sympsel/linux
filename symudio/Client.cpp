#include "Socket.hpp"

void test1() {
    Socket clientSocket;
    clientSocket.createClient(8080, "127.0.0.1");
    std::string str = "avgdrstfbgcftg";
    clientSocket.send(str.data(), str.size());
    char buffer[1024];
    buffer[0] = '\0';
    clientSocket.recv(buffer, 1024 - 1);
    LOG_DEBUG() << std::format("recv: {}", buffer);
    clientSocket.close();
}

int main() {
    test1();
    return 0;
}
