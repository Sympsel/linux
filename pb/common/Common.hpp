#pragma once

#include "add_contact.pb.h"

template <>
struct std::formatter<add_contact::Request> {
    // 1. constexpr parse 方法（接受格式解析上下文）
    constexpr auto parse(const std::format_parse_context& ctx) {
        return ctx.begin();
    }

    // 2. format 方法（接受对象和格式化上下文）
    auto format(const add_contact::Request& obj, std::format_context& ctx) const {
        std::ostringstream oss;
        int indent_level = 0;

        auto indent = [&](const int level) {
            for (int i = 0; i < level; ++i) {
                oss << "  ";  // 2空格缩进
            }
        };

        oss << "Request {\n";
        indent_level = 1;

        // name 字段
        indent(indent_level);
        oss << "name: \"" << obj.name() << "\"\n";

        // age 字段
        indent(indent_level);
        oss << "age: " << obj.age() << "\n";

        // phone 列表
        if (obj.phone_size() > 0) {
            indent(indent_level);
            oss << "phone: [\n";
            for (int i = 0; i < obj.phone_size(); ++i) {
                const auto& phone = obj.phone(i);
                indent(indent_level + 1);
                oss << "[" << i << "] {\n";

                indent(indent_level + 2);
                oss << "number: \"" << phone.number() << "\"\n";

                indent(indent_level + 1);
                oss << "}\n";
            }
            indent(indent_level);
            oss << "]\n";
        } else {
            indent(indent_level);
            oss << "phone: []\n";
        }

        // address 对象
        if (obj.has_address()) {
            const auto& addr = obj.address();
            indent(indent_level);
            oss << "address: {\n";

            indent(indent_level + 1);
            oss << "home_address: \"" << addr.home_address() << "\"\n";

            indent(indent_level + 1);
            oss << "unit_address: \"" << addr.unit_address() << "\"\n";

            indent(indent_level);
            oss << "}\n";
        } else {
            indent(indent_level);
            oss << "address: null\n";
        }

        indent(0);
        oss << "}";

        std::string result = oss.str();
        return std::format_to(ctx.out(), "{}", result);
    }
};