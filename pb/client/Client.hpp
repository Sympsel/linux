#pragma once


#include <iostream>
#include <print>

#include "httplib.h"

#include "add_contact.pb.h"
#include "ContactException.hpp"
#include "ReadUtils.hpp"

// 该状态与某宏冲突
#ifdef ADD
#undef ADD
#endif

class Client {
private:
    enum class Status {
        NORMAL,
        ADD,
        DEL,
        LIST,
        DETAIL,
        EXIT,
        UNKNOWN
    };

    static void showMenu() {
        std::println("== 请选择对联系人列表的操作 ==");
        std::println("== 1、新增 ==");
        std::println("== 2、删除 ==");
        std::println("== 3、查看列表 ==");
        std::println("== 4、查看详细信息 ==");
        std::println("== 5、退出 ==");
        std::println("=============================");
    }

    void startJustNow() {
        std::println("============= 通讯录 =============");
        _status = Status::NORMAL;
    }

    static void buildPeopleInfo(add_contact::Request &req) {
        std::println("=== 开始录入 ===");
        std::string name;

        // 在选择1后会跳过
        ReadUtils::ignoreUntil();
        std::print("请输入姓名: ");
        ReadUtils::readLine(name);
        req.set_name(name);

        std::print("请输入年龄: ");
        const int age = ReadUtils::readIntWithRetry();
        ReadUtils::ignoreUntil();  // 忽略年龄后的换行符
        req.set_age(age);

        int i = 0;
        std::println("依次输入电话号码，输入单个 0 退出");
        while (true) {
            std::print("编号{}：", i);
            std::string number;
            ReadUtils::readLine(number);
            if (!number.empty() && number == "0") {
                break;
            }
            auto *a = req.add_phone();
            a->set_number(number);
            ++i;
        }

        add_contact::Request_Address address;
        std::string home_addr;
        std::string unit_addr;
        std::print("请输入家庭地址：");
        ReadUtils::readLine(home_addr);
        address.set_home_address(home_addr);
        std::print("请输入单位地址：");
        ReadUtils::readLine(unit_addr);
        address.set_unit_address(unit_addr);
        *req.mutable_address() = address;

        std::println("=== 停止录入 ===");
    }

    void addNewItem() {
        std::println("添加联系人");
        httplib::Client http_client{_serverIp, _serverPort};

        // 构造请求
        add_contact::Request req;
        buildPeopleInfo(req);
        // 序列化请求
        std::string req_str;
        if (!req.SerializeToString(&req_str)) {
            throw ContactException("程序序列化请求出错");
        }

        // 向服务端发起Post调用
        auto http_resp = http_client.Post(
            "/contact/add", req_str, "application/protobuf"
        );
        if (!http_resp) {
            throw ContactException(std::format(
                "请求失败：{}，错误信息：{}",
                "/contact/add", httplib::to_string(http_resp.error())
            ));
        }

        // 反序列化响应
        add_contact::Response resp;
        if (!resp.ParseFromString(http_resp->body)) {
            throw ContactException(std::format(
                "请求失败：{}，解析响应失败",
                "/contact/add"
            ));
        }

        if (http_resp->status != 200) {
            throw ContactException(std::format(
                "请求失败：{}，HTTP状态码：{}，错误原因：{}",
                "/contact/add", http_resp->status, resp.error_desc()
            ));
        }

        if (!resp.success()) {
            throw ContactException(std::format(
                "请求失败：{}，结果异常，原因：{}",
                "/contact/add", resp.error_desc()
            ));
        }
        std::println("成功添加联系人，uid：{}", resp.uid());
        _status = Status::NORMAL;
    }

    void delAnItem() {
        std::println("删除联系人");
        _status = Status::NORMAL;
    }

    void showAllItems() {
        std::println("联系人列表");
        _status = Status::NORMAL;
    }

    void showItemDetails() {
        std::println("联系人详细信息");
        _status = Status::NORMAL;
    }

    void exitContact() {
        std::println("============= 退出 =============");
        _status = Status::EXIT;
    }

    void handleInputError() {
        std::println("输入错误，请重新输入");
        _status = Status::NORMAL;
    }

public:
    /**
     * @warning you mush setup server Ip and Port before Run, call setServer(ip, port)
     */
    void run() {
        if (_serverPort == 0 || _serverIp.empty()) {
            throw ContactException("致命错误：没有设置IP和端口");
        }

        std::println("==== 通讯录 ====");

        while (_status != Status::EXIT) {
            try {
                switch (_status) {
                    case Status::NORMAL: {
                        showMenu();
                        int op;
                        std::print("User Input> ");
                        std::cin >> op;
                        if (op == 1) {
                            _status = Status::ADD;
                        } else if (op == 2) {
                            _status = Status::DEL;
                        } else if (op == 3) {
                            _status = Status::LIST;
                        } else if (op == 4) {
                            _status = Status::DETAIL;
                        } else if (op == 5) {
                            _status = Status::EXIT;
                        } else {
                            _status = Status::UNKNOWN;
                        }
                        break;
                    }
                    case Status::ADD:
                        addNewItem();
                        break;
                    case Status::DEL:
                        delAnItem();
                        break;
                    case Status::LIST:
                        showAllItems();
                        break;
                    case Status::DETAIL:
                        showItemDetails();
                        break;
                    case Status::UNKNOWN:
                        handleInputError();
                        break;
                    default:
                        break;
                }
            } catch (const ContactException &e) {
                std::println("捕获到异常\n\t----{}", e.what());
                _status = Status::NORMAL;
            }
        }
        exitContact();
    }

    void setServer(std::string serverIp, const int serverPort) {
        _serverIp = std::move(serverIp);
        _serverPort = serverPort;
    }

private:
    Status _status = Status::NORMAL;
    std::string _serverIp;
    int _serverPort{};
};
