#include <iostream>

#include "src/HttpServer.hpp"

void Usage(const std::string &proc_name) {
    std::cout << "Usage: " << proc_name << " <port>" << std::endl;
}

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    in_port_t server_port = std::stoi(argv[1]);

    const auto http_server = std::make_unique<HttpServer>(server_port);

    http_server->Run([](const HttpRequest& req) -> HttpResponse {
        LOG_INFO() << "get a request: " << req.method << " " << req.path << " " << req.version;
        HttpResponse response;
        response.status_code = "200";
        response.status_text = "OK";
        response.headers["Content-Type"] = "text/html";
        response.headers["Server"] = "SymNet/1.0";

        if (req.path == "/" || req.path == "/index.html") {
            response.body = "<html><body><h1>Hello, World!</h1></body></html>";
        } else {
            response.status_code = "404";
            response.status_text = "Not Found";
            response.body = "<html><body><h1>404 Not Found</h1></body></html>";
        }
        return response;
    });

    return 0;
}
