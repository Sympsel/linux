#include <functional>
#include <iostream>

#include "src/TcpServer.hpp"
#include "src/CalcTypes.hpp"
#include "src/JsonSerializer.hpp"

#include "../../utils/include/snet/Protocol.hpp"

// 业务层任务逻辑
using calc_task = std::function<CalcResponse(const CalcRequest&)>;
// 网络层任务逻辑
using net_task = std::function<std::string(const std::string &)>;

void Usage(const std::string &proc_name) {
    std::cout << "Usage: " << proc_name << " [port]" << std::endl;
}

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        exit(1);
    }

    // Deamon();

    in_port_t server_port = std::stoi(argv[1]);

    LOG_INFO() << "=== Calculator Server Starting ===";
    LOG_INFO() << "Port: " << server_port;

    JsonSerializer jsonSerializer;
    const auto protocol = std::make_unique<ProtoCol<calc_task, JsonSerializer>>(
        [](const CalcRequest &req) -> CalcResponse {
            // 业务逻辑
            CalcResponse resp;

            switch (req.oper) {
                case '+':
                    resp.result = req.x + req.y;
                    resp.status = CalcStatus::SUCCESS;
                    break;
                case '-':
                    resp.result = req.x - req.y;
                    resp.status = CalcStatus::SUCCESS;
                    break;
                case '*':
                    resp.result = req.x * req.y;
                    resp.status = CalcStatus::SUCCESS;
                    break;
                case '/':
                    if (req.y == 0) {
                        resp.status = CalcStatus::DIV_ZERO;
                    } else {
                        resp.result = req.x / req.y;
                        resp.status = CalcStatus::SUCCESS;
                    }
                    break;
                default:
                    resp.status = CalcStatus::UNKNOWN_OP;
                    break;
            }

            return resp;
        },
        jsonSerializer  // 注入序列化器
    );
    LOG_INFO() << "Protocol layer initialized with JsonSerializer";

    // 创建网络层
    const auto tcp_server = std::make_shared<TcpServer<net_task>>(server_port);
    // 注入协议处理函数
    tcp_server->Init([&protocol](const std::string &req) -> std::string {
        return protocol->HandleRequest(req);
    });
    LOG_INFO() << "TCP Server initialized, starting...";
    tcp_server->Run();

    return 0;
}
