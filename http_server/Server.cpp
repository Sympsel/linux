#include <iostream>

#include "src/app/Application.hpp"

void Usage(const std::string &proc) {
    std::cout << "Usage: " << proc << " <port>" << std::endl;
}

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        return -1;
    }

    try {
        // 初始化路径管理器
        auto &path_mgr = PathManager::getInstance();
        // /proc/self/exe: 获取当前进程的绝对路径
        path_mgr.initialize(fs::canonical("/proc/self/exe"));

        int port = std::stoi(argv[1]);

        LOG_INFO() << "Starting HTTP server on port " << port;

        const auto server = std::make_unique<HttpServer>(port);
        auto app = std::make_unique<Application>();

        app->registerHandles();

        server->setHttpCallback(
            [&app](HttpRequest &req, HttpResponse &resp) {
                app->handleRequest(req, resp);
            }
        );

        server->start();
    } catch (const std::exception &e) {
        LOG_FATAL() << "Server error: " << e.what();
        return -1;
    }

    return 0;
}
