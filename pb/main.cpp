#include <iostream>
#include "utils/httplib.h"

using std::cout;
using std::endl;
using namespace httplib;

int main() {
    cout << "==== 服务启动 ====" << endl;
    Server server;

    server.Post("/test-post", [](const Request& req, Response& resp) {
        cout << "接收到post请求" << endl;
        resp.status = 200;
    });

    server.Get("/test-get", [](const Request& req, Response& resp) {
        cout << "接收到get请求" << endl;
        resp.status = 200;
    });

    server.listen("0.0.0.0", 8123);
    return 0;
}