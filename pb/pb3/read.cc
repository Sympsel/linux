#include <iostream>
#include <fstream>
#include "contacts.pb.h"

void PrintContacts(const contacts::Contacts &contacts) {
    for (int i = 0; i < contacts.contacts_size(); ++i) {
        std::cout << "联系人信息：{" << std::endl;
        contacts::PeopleInfo people = contacts.contacts(i);
        std::cout << "  姓名：" << people.name() << std::endl;
        std::cout << "  年龄：" << people.age() << std::endl;
        std::cout << "  电话号码：[" << std::endl;
        for (int j = 0; j < people.phone_size(); ++j) {
            auto phone = people.phone(j);
            std::cout << "    " << j << ": " << phone.number() << std::endl;
        }
        std::cout << "  ]" << std::endl;
        if (people.has_data() && people.data().Is<contacts::Address>()) {
            contacts::Address address;
            (void) people.data().UnpackTo(&address);
            if (!address.home_address().empty()) {
                std::cout << "  家庭地址：" << address.home_address() << std::endl;
            }
            if (!address.unit_address().empty()) {
                std::cout << "  单位地址：" << address.unit_address() << std::endl;
            }
        }
        switch (people.other_contact_case()) {
            case contacts::PeopleInfo::OtherContactCase::kQq:
                std::cout << "  QQ：" << people.qq() << std::endl;
                break;
            case contacts::PeopleInfo::OtherContactCase::kWechat:
                std::cout << "  微信：" << people.wechat() << std::endl;
                break;
            default:
                break;
        }
        if (people.remark_size()) {
            std::cout << "  备注：[" << std::endl;
            for (auto it = people.remark().cbegin(); it != people.remark().cend(); ++it) {
                std::cout << "    " << it->first << ": " << it->second << std::endl;
            }
            for (int j = 0; j < people.remark_size(); ++j) {
            }
            std::cout << "  ]" << std::endl;
        }
        std::cout << "}" << std::endl;
    }
}

int main() {
    std::fstream input("contacts.bin", std::ios::in | std::ios::binary);
    contacts::Contacts contacts;
    if (!input) {
        std::cout << "contacts.bin not found" << std::endl;
    } else if (!contacts.ParseFromIstream(&input)) {
        std::cerr << "parse error!" << std::endl;
        input.close();
        return -1;
    }
    PrintContacts(contacts);
    return 0;
}
