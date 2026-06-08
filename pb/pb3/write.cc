#include <iostream>
#include <fstream>

#include "contacts.pb.h"


#define CIN_IGN(a_char) std::cin.ignore(128, a_char)
#define NEWLINE_IGN std::cin.ignore(128, '\n')
#define GET_LINE(str_to_fill) std::getline(std::cin, str_to_fill)

void AddPeopleInfo(contacts::PeopleInfo *people) {
    std::cout << "=== 开始录入 ===" << std::endl;
    std::string name;
    std::cout << "请输入姓名: ";
    GET_LINE(name);
    people->set_name(name);
    int age;
    std::cout << "请输入年龄: ";
    std::cin >> age;
    people->set_age(age);
    NEWLINE_IGN;
    int i = 0;
    std::cout << "依次输入编号，输入单个0退出" << std::endl;
    while (true) {
        std::cout << "编号" << i << ": ";
        std::string number;
        GET_LINE(number);
        if (!number.empty() && number == "0") {
            break;
        }
        auto *a = people->add_phone();
        a->set_number(number);
        ++i;
    }
    contacts::Address address;
    std::string home_addr;
    std::string unit_addr;
    NEWLINE_IGN;
    std::cout << "请输入家庭地址：";
    GET_LINE(home_addr);
    std::cout << "请输入单位地址：";
    GET_LINE(unit_addr);
    address.set_unit_address(unit_addr);

    (void) people->mutable_data()->PackFrom(address);

    std::cout << "请选择要添加的其他联系方式（0.直接添加 | 1.QQ | 2.微信）：";
    int op;
    bool retype = true;
    std::string other_contact;
    while (retype) {
        retype = false;
        std::cin >> op;
        switch (op) {
            case 0:
                break;
            case 1:
                std::cout << "请输入QQ号：";
                GET_LINE(other_contact);
                people->set_qq(other_contact);
                break;
            case 2:
                std::cout << "请输入微信号：";
                GET_LINE(other_contact);
                people->set_wechat(other_contact);
                break;
            default:
                std::cout << "错误的选项";
                retype = true;
        }
    }

    std::cout << " ==开始录入备注==" << std::endl;
    for (int j = 0; ; ++j) {
        std::cout << "备注编号" << " " << j << ", " << "请输入备注名：";
        std::string remark_key;
        GET_LINE(remark_key);
        if (remark_key.empty()) {
            std::cout << " ==停止录入备注==" << std::endl;
            break;
        }
        std::cout << "请输入备注：";
        std::string remake_value;
        GET_LINE(remake_value);
        people->mutable_remark()->insert({
            remark_key, remake_value
        });
    }

    std::cout << "=== 停止录入 ===" << std::endl;
}

int main() {
    std::fstream input("contacts.bin", std::ios::in | std::ios::binary);
    contacts::Contacts contacts;
    if (!input) {
        std::cout << "contacts.bin not found, created!" << std::endl;
    } else if (!contacts.ParseFromIstream(&input)) {
        std::cerr << "parse error!" << std::endl;
        input.close();
        return -1;
    }

    AddPeopleInfo(contacts.add_contacts());

    std::fstream output("contacts.bin", std::ios::out | std::ios::binary | std::ios::trunc);
    if (!contacts.SerializePartialToOstream(&output)) {
        std::cerr << "serialize error!" << std::endl;
        input.close();
        output.close();
        return -1;
    }
    std::cout << "write successfully!" << std::endl;
    input.close();
    output.close();

    return 0;
}
