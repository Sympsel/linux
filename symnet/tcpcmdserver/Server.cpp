#include <iostream>
#include <memory>

#include "src/ExecuteCommand.hpp"
#include "src/TcpServer.hpp"

void Usage(const std::string& proc_name) {
    std::cout << "Usage: " << proc_name << " [port]" << std::endl;
}

void WhiteList(ExecuteCommandServer* execute_command_server) {
    execute_command_server->Init();
    execute_command_server->AddToWhiteList("ls -a");
    execute_command_server->AddToWhiteList("ls -l");
    execute_command_server->AddToWhiteList("ls -al");
    execute_command_server->AddToWhiteList("pwd");
}

// for demo
int main(const int argc, char* argv[]) {
    if (argc != 2) {
        Usage(argv[0]);
        exit(1);
    }
    in_port_t server_port = std::stoi(argv[1]);
    auto execute_command_server = std::make_unique<ExecuteCommandServer>();
    WhiteList(execute_command_server.get());
    const auto server = std::make_unique<TcpServer<exec_cmd_t>>(server_port);
    server->Init([&execute_command_server](const std::string& command_str) -> std::string {
        return execute_command_server->Execute(command_str);
    });
    server->Start();
    return 0;
}