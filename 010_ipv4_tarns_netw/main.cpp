#include <arpa/inet.h>

#include <cstdint>
#include <iostream>
#include <sstream>

using std::cout;
using std::endl;
using std::string;

inline bool islittleendian() {
    union {
        int i;
        char c;
    } un;
    un.i = 1;
    return un.c == 1;
}

struct ipv4_octets {
    uint8_t o1, o2, o3, o4;  // 语义化命名，始终按"点分十进制从左到右"存储
};

// 点分十进制字符串 → 四元组
ipv4_octets parse_ipv4(const string& ip_str) {
    ipv4_octets ret{};
    char dot;
    std::istringstream ss(ip_str);
    int v1, v2, v3, v4;  // 先用 int 接收，再转 uint8_t
    ss >> v1 >> dot >> v2 >> dot >> v3 >> dot >> v4;
    ret.o1 = static_cast<uint8_t>(v1);
    ret.o2 = static_cast<uint8_t>(v2);
    ret.o3 = static_cast<uint8_t>(v3);
    ret.o4 = static_cast<uint8_t>(v4);
    return ret;
}

// 四元组 → 点分十进制字符串
string ipv4_to_string(const ipv4_octets& ip) {
    return std::to_string(ip.o1) + '.' +
           std::to_string(ip.o2) + '.' +
           std::to_string(ip.o3) + '.' +
           std::to_string(ip.o4);
}

// 四元组 → in_addr_t（网络字节序，大端）
in_addr_t ipv4_to_network(const ipv4_octets& ip) {
    in_addr_t order = static_cast<in_addr_t>(ip.o1) << 24 |
                      static_cast<in_addr_t>(ip.o2) << 16 |
                      static_cast<in_addr_t>(ip.o3) << 8 |
                      static_cast<in_addr_t>(ip.o4);
    return htonl(order);
}

// in_addr_t（网络字节序） → 点分十进制字符串
string network_to_string(in_addr_t ip) {
    uint32_t host_ip = ntohl(ip);  // 网络序转主机序
    return std::to_string((host_ip >> 24) & 0xFF) + '.' +
           std::to_string((host_ip >> 16) & 0xFF) + '.' +
           std::to_string((host_ip >> 8) & 0xFF) + '.' +
           std::to_string(host_ip & 0xFF);
}

// 更标准的方式：用 ntohl + 位运算（可移植，不依赖指针类型转换）
string network_to_string_safe(in_addr_t ip) {
    uint32_t h = ntohl(ip);  // 转主机字节序，然后统一按小端解析
    return std::to_string((h >> 24) & 0xFF) + '.' +
           std::to_string((h >> 16) & 0xFF) + '.' +
           std::to_string((h >> 8) & 0xFF) + '.' +
           std::to_string(h & 0xFF);
}

void test() {
    ipv4_octets ip = parse_ipv4("127.0.0.1");
    cout << ipv4_to_string(ip) << endl;  // 127.0.0.1

    in_addr_t net = ipv4_to_network(ip);
    cout << network_to_string(net) << endl;       // 127.0.0.1
    cout << network_to_string_safe(net) << endl;  // 127.0.0.1

    // 测试边界值
    ipv4_octets ip2 = parse_ipv4("255.192.168.1");
    cout << ipv4_to_string(ip2) << endl;                      // 255.192.168.1
    cout << network_to_string(ipv4_to_network(ip2)) << endl;  // 255.192.168.1
}

int main() {
    test();
    return 0;
}
