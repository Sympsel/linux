#include <iostream>

#include "JsonSerializer.hpp"
#include "src/TcpServer.hpp"
#include "../utils/module/SymNet.h"

void Usage(const std::string &proc_name) {
    std::cout << "Usage: " << proc_name << "<ip> <port>" << std::endl;
}

int main(const int argc, char *argv[]) {
    if (argc != 3) {
        Usage(argv[0]);
        exit(1);
    }
    const std::string server_ip = argv[1];
    const in_port_t server_port = std::stoi(argv[2]);
    const InetAddr server_addr(server_port, server_ip);

    // 创建客户端套接字
    const auto client_socket = std::make_unique<TcpSocket>();
    client_socket->BuildClientSocketMethod();

    if (!client_socket->Connect(server_addr)) {
        LOG_ERROR() << "Connect to server failed";
        exit(EXIT_FAILURE);
    }

    std::cout << "Connect to server success, at[" << server_addr.GetIp() << ":" << server_addr.GetPort() << "]" <<
            std::endl;
    std::cout << "Enter calculations in a line (e.g., '10 + 20'), or 'quit' to exit." << std::endl;

    // 创建协议层
    JsonSerializer json_serializer;
    // 占位任务, 用于自动推导类型
    const auto dummy_task = [](const CalcRequest &) -> CalcResponse {
        return CalcResponse{};
    };
    const auto protocol = std::make_unique<ProtoCol<decltype(dummy_task), JsonSerializer> >(
        dummy_task,
        json_serializer
    );
    std::string line;
    while (std::cout << "> ", std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (line == "quit" || line == "exit") {
            break;
        }

        int x, y;
        char op = '\0';
        std::stringstream ss(line);
        ss >> x >> op >> y;
        if (ss.fail()) {
            std::cout << "Invalid input. Use format: '10 + 20'";
            continue;
        }
        // 构造请求
        CalcRequest request(x, y, op);
        // 序列化请求
        std::string json_str = json_serializer.SerializeRequest(request);

        // 打包并发送请求
        if (const std::string send_str = protocol->Package(json_str); !client_socket->Send(send_str)) {
            LOG_ERROR() << "Failed to send request";
            break;
        }

        std::string recv_str;
        if (const ssize_t n = client_socket->Recv(recv_str); n <= 0) {
            LOG_ERROR() << "Failed to receive response";
            break;
        }

        // 解包响应
        std::string response_json;
        if (protocol->Unpackage(recv_str, response_json) <= 0) {
            LOG_ERROR() << "Failed to unpackage response";
            continue;
        }

        // 反序列化响应
        CalcResponse response;
        if (!json_serializer.DeserializeResponse(response_json, response)) {
            LOG_ERROR() << "Failed to deserialize response";
        }

        // 显示结果
        if (response.status == CalcStatus::SUCCESS) {
            std::cout << "Result: " << x << " " << op << " " << y
                    << " = " << response.result << std::endl;
        } else {
            std::cerr << "Error: ";
            switch (response.status) {
                case CalcStatus::DIV_ZERO:
                    std::cerr << "Division by zero";
                    break;
                case CalcStatus::MOD_ZERO:
                    std::cerr << "Modulo by zero";
                    break;
                case CalcStatus::UNKNOWN_OP:
                    std::cerr << "Unknown operator '" << op << "'";
                    break;
                default:
                    std::cerr << "Calculation failed";
                    break;
            }
            std::cerr << std::endl;
        }
    }

    return 0;
}
