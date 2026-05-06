#include "UdpServer.hpp"
#include "User.hpp"
#include "ThreadPool.hpp"
#include "RouteServer.hpp"
#include "Config.hpp"

using task_t = std::function<void()>;

static void Usage(const std::string& name) {
    std::cerr << "Usage: " << name << " <port>" << std::endl;
}


// static void initConf() {
//      Conf::conf_instance.Get("style", "system_name", "SYSTEM")
// }

int main(const int argc, char* argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        exit(1);
    }

    USE_CONSOLE_LOG();
    // initConf();

    in_port_t server_port = std::stoi(argv[1]);

    /*
       we must launch the udp-server before clients send message to server,
       or clients won't receive message until itself send a message to server
    */
    const auto udpServer = std::make_unique<UdpServer>(
        [](const std::string& message, const User& who, int sockfd) {
            ThreadPool<task_t>::GetInstance().Enqueue([message, who, sockfd]() -> void {
                RouteServer::GetInstance().RouteMessage(message, who, sockfd);
            });
        }, server_port);
    udpServer->Init();
    udpServer->Start();

    return 0;
}