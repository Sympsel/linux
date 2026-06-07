#pragma once

#include "StaticFileHandler.hpp"
#include "../http/HttpServer.hpp"
#include "../utils/PathManager.hpp"
#include "../utils/FileReader.hpp"

namespace http_v {
    inline constexpr std::string args_spilt = "?";
}

class Application {
    using HandleRoute = std::function<void(const HttpRequest &, HttpResponse &)>;

private:


    bool NeedHandle(const std::string &key) const {
        return _routes.contains(key);
    }

    void registerHandle(const std::string &api_path, const HandleRoute &handle_cb) {
        _routes[api_path] = handle_cb;
    }

public:
    Application() = default;

    ~Application() = default;


    void handleRequest(HttpRequest &request, HttpResponse &response) {
        using namespace http_v;
        if (request.method == HttpMethod::GET) {
            const size_t pos = request.path.find(args_spilt);
            if (pos != std::string::npos) {
                args = request.path.substr(pos + args_spilt.size());
                request.path = request.path.substr(0, pos);
            }
        }

        // 优先匹配 API 路由
        if (NeedHandle(request.path)) {
            request.print(args);
            _routes[request.path](request, response);
            response.print();
            return;
        }


        request.print();

        // 根路径和静态文件
        StaticFileHandle::serveStaticFile(request, response);
        response.print();
    }

    void registerHandles() {
        registerHandle("/api/login", [](const HttpRequest &req, HttpResponse &resp) {
            std::cout << "Login Server..." << std::endl;
        });
        registerHandle("/api/users", [](const HttpRequest &req, HttpResponse &resp) {
            resp.status_code = 200;
            resp.status_text = "OK";
            resp.body = R"({"users": ["Alice", "Bob", "Charlie"]})";
            resp.setHeader("Content-Type", "application/json");
        });

        registerHandle("/api/echo", [](const HttpRequest &req, HttpResponse &resp) {
            resp.status_code = 200;
            resp.status_text = "OK";
            resp.body = "Echo: " + req.body;
            resp.setHeader("Content-Type", "text/plain");
            resp.setHeader("Content-Length", std::to_string(resp.body.size()));
        });
    }

private:
    std::string args;
    std::unordered_map<std::string, HandleRoute> _routes;
};
