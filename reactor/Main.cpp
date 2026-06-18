#include <functional>
#include <memory>

#include "core/Connection.hpp"
#include "http/HttpConnection.h"
#include "network/Listener.h"
#include "Reactor.hpp"

int main() {
    // 创建 HTTP 协议的工厂函数
    auto httpFactory = [](int fd) -> std::shared_ptr<Connection> {
        auto conn = std::make_shared<HttpConnection>(fd);

        // 设置 HTTP 请求处理器（在转换之前）
        conn->setRequestHandler([](const HttpRequest &req, HttpResponse &resp) {
            // 业务逻辑
            resp._msg.statusCode = 200;
            resp._msg.statusText = "OK";
            resp._msg.headers["Content-Type"] = "text/html";
            resp._msg.body = "<h1>Hello World</h1>";
        });

        // 返回基类指针
        return conn;
    };

    const auto listener = std::make_shared<Listener>(8080, httpFactory);
    const auto reactorServer = std::make_unique<Reactor>();
    reactorServer->addListener(listener);
    reactorServer->dispatcher();

    return 0;
}