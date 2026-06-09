#include <print>

#include "httplib.h"
#include "Common.hpp"
#include "ContactException.hpp"

static unsigned int random_char() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);
    return dis(gen);
}

static std::string generate_hex(const unsigned int len) {
    std::stringstream ss;
    for (int i = 0; i < len; ++i) {
        const auto rc = random_char();
        std::stringstream hex_ss;
        hex_ss << std::hex << rc;
        auto hex = hex_ss.str();
        ss << (hex.length() == 2 ? hex : '0' + hex);
    }
    return ss.str();
}

int main() {
    std::println("==== 服务启动 ====");
    httplib::Server server;

    server.Post("/contact/add", [](
            const httplib::Request &http_req, httplib::Response &http_resp) {
                    add_contact::Request req;
                    add_contact::Response resp;
                    try {
                        std::println("接收到post请求: {}", "/contact/add");
                        // 反序列化
                        if (!req.ParseFromString(http_req.body)) {
                            throw ContactException(std::format(
                                "添加到通讯列表出错：错误原因{}", "反序列化请求出错"
                            ));
                        }
                        // 新增联系人
                        std::println("{}", req);

                        // todo 存储

                        // 构造响应
                        resp.set_uid(generate_hex(10));
                        resp.set_success(true);
                        resp.set_uid("aaaa");

                        std::string resp_str;
                        if (!resp.SerializeToString(&resp_str)) {
                            throw ContactException(std::format(
                                "添加到通讯列表出错：错误原因{}", "序列化响应出错"
                            ));
                        }
                        http_resp.body = resp_str;
                        http_resp.set_header("Content-Type", "application/protobuf");
                        http_resp.status = 200;
                    } catch (const ContactException &e) {
                        http_resp.status = 500;
                        resp.set_success(false);
                        resp.set_error_desc(e.what());
                        std::string resp_str;
                        if (resp.SerializeToString(&resp_str)) {
                            http_resp.body = resp_str;
                            http_resp.set_header("Content-Type", "application/protobuf");
                        }
                        std::println("捕获了一个异常: {}", e.what());
                    }
                }
    );

    server.Get("/test-get", [](const httplib::Request &req, httplib::Response &resp) {
        std::println("接收到get请求");
        resp.status = 200;
    });

    server.listen("0.0.0.0", 8123);
    return 0;
}
