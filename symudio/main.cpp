#include <print>
#include <regex>

#include "entity/Any.hpp"

class Test {
public:
    Test() {
        std::println("构造");
    }

    Test(const Test& test) {
        std::println("拷贝构造");
    }

    ~Test() {
        std::println("析构");
    }
};

void delTest(const Test *t) {
    delete t;
}

int main() {
    // std::string str = "/numbers/1234";
    // std::regex reg{"/numbers/(\\d+)"};
    // std::smatch sm;
    // bool ret = std::regex_match(str, sm, reg);
    // if (ret) {
    //     for (auto &match : sm) {
    //         std::println("{}", match.str());
    //     }
    // } else {
    //     std::println("匹配错误");
    //     return -1;
    // }

    // std::string httpReq = "GET /numbers/login?user=sym&password=123456 HTTP/1.1\r\n";
    // std::regex reg("(GET|HEAD|matchPOST|PUT|DELETE) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)?");
    // std::smatch sm;
    // bool ret = std::regex_match(httpReq, sm, reg);
    // if (ret) {
        // for (auto match : sm) {
            // std::println("{}", match.str());
        // }
    // }

    Any a;
    a = 10;
    if (int *pa = a.get<int>()) {
        std::println("int value: {}", *pa);
    }
    a = "aaa";
    if (std::string* s = a.get<std::string>()) {
        std::println("string value: {}", *s);
    }
    // Any a;
    // {
        // Test test;
        // a = test;
    // }
    return 0;
}