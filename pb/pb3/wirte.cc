#include <iostream>
#include <fstream>

#include "contacts.pb.h"

void AddPeopleInfo(contacts::PeopleInfo* people) {
    // name age
    std::cout << "===" << std::endl;
    std::string name;
    std::getline(std::cin, name);
    int age;
    std::cin >> age;
    people->set_age(age);
    std::cin.ignore(256, '\n');
    int i = 0;
    while (true) {
        std::cout << i << ": ";
        std::string number;
        std::getline(std::cin, number);
        if (number.empty()) {
            break;
        }
        auto* a = people->add_phone();
        a->set_number(number);
        ++i;
    }
    std::cout << "===" << std::endl;
}

int main() {
    std::fstream input("contacts.bin", std::ios::in | std::ios::binary);
    contacts::Contacts contacts;
    if (!input) {
        std::cout << "contacts.bin not found, create new file!" << std::endl;
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