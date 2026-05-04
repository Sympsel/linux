#include "UdpServer.hpp"
#include "User.hpp"
#include "ThreadPool.hpp"
#include "Route.hpp"

using task_t = std::function<void()>;

inline void Usage(const std::string& name) {
    std::cerr << "Usage: " << name << " <port>" << std::endl;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        exit(1);
    }

    USE_CONSOLE_LOG();

    in_port_t server_port = std::stoi(argv[1]);

    auto& r = Route::GetInstance();
    std::unique_ptr<UdpServer> usvr = std::make_unique<UdpServer>(
        [&r](const std::string& message, const InetManager& who, int sockfd) {
            ThreadPool<task_t>::GetInstance().Enqueue([&r, &message, &who, &sockfd]() -> void {
                r.RouteMessage(message, who, sockfd);
            });
        }, server_port);
    usvr->Init();
    usvr->Start();

    return 0;
}