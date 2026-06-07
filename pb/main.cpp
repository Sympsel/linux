#include <iostream>

#include "contacts.pb.h"

int main() {
    std::string pif_s;
    {
        contacts::PeopleInfo pif;
        pif.set_name("zhangsan");
        pif.set_age(18);
        (void)pif.SerializeToString(&pif_s);
        std::cout << pif_s << std::endl;
    }

    {
        contacts::PeopleInfo pif;
        (void)pif.ParseFromString(pif_s);
        std::cout << pif.name() << " " << pif.age() << std::endl;
    }
    return 0;
}
