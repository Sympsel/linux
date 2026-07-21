#include <print>
#include <regex>

#include "EventLoop.hpp"
#include "entity/Any.hpp"

class Test {
public:
    Test() {
        std::println("构造");
    }

    Test(const Test &test) {
        std::println("拷贝构造");
    }

    ~Test() {
        std::println("析构");
    }
};

void delTest(const Test *t) {
    delete t;
}

void test2() {
    Any a;
    a = 10;
    if (int *pa = a.get<int>()) {
        std::println("int value: {}", *pa);
    }
    a = "aaa";
    if (std::string *s = a.get<std::string>()) {
        std::println("string value: {}", *s);
    }
    // Any a;
    // {
    // Test test;
    // a = test;
    // }
}

void test1() {
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
}

#include "Socket.hpp"

void test3() {
    Socket listenSocket;
    listenSocket.createServer(8080);
    while (true) {
        int newFd = listenSocket.accept();
        if (newFd < 0) {
            LOG_ERROR() << "accept error";
            continue;
        }
        Socket clientSocket(newFd);
        char buffer[1024];
        buffer[0] = '\0';
        ssize_t ret = clientSocket.recv(buffer, 1024 - 1);
        if (ret < 0) {
            clientSocket.close();
            continue;
        }
        clientSocket.send(buffer, ret);
        clientSocket.close();
    }
    listenSocket.close();
}

void test4() {
    EventLoop loop;
}

int main() {
    // test3();
    test4();
    return 0;
}
