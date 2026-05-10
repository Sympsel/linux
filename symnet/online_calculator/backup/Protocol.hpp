#pragma once

#include <json/json.h>
#include "../utils/Log.hpp"

// class Request {
// public:
//     Request() : _x(), _y(), _oper('+') {
//     }
//
//     Request(const int x, const int y, const char oper)
//         : _x(x), _y(y), _oper(oper) {
//     }
//
//     void Serialize(std::string &str_to_fill) const {
//         Json::Value root;
//         root["data_x"] = _x;
//         root["data_y"] = _y;
//         // 把操作符转换成字符串,否则会被当int处理, 导致后续读取的时候读取了整数的最高位
//         root["oper"] = std::string(1, _oper);
//
//         Json::FastWriter writer;
//         str_to_fill = writer.write(root);
//     }
//
//     void Deserialize(const std::string &str) {
//         Json::Value root;
//         if (Json::Reader reader; !reader.parse(str, root)) {
//             LOG_ERROR() << "parse error: " << reader.getFormattedErrorMessages();
//             return;
//         }
//         _x = root["data_x"].asInt();
//         _y = root["data_y"].asInt();
//         _oper = root["oper"].asString()[0];
//     }
//
//     [[nodiscard]] const char &GetOper() const {
//         return _oper;
//     }
//
//     [[nodiscard]] const int &GetX() const {
//         return _x;
//     }
//
//     [[nodiscard]] const int &GetY() const {
//         return _y;
//     }
//
//     ~Request() = default;
//
// private:
//     int _x;
//     int _y;
//     char _oper;
// };
//
// class Response {
// public:
//     enum ExitCode {
//         SUCCESS = 0,
//         MAY_BE_ERROR = 1,
//         ERROR = 2
//     };
//
//     Response() : _result(0), _exitcode(ERROR) {
//     }
//
//     explicit Response(const int result)
//         : _result(result), _exitcode(MAY_BE_ERROR) {
//     }
//
//     void Serialize(std::string &str_to_fill) const {
//         Json::Value root;
//         root["result"] = _result;
//         root["exitcode"] = _exitcode;
//
//         Json::FastWriter writer;
//         str_to_fill = writer.write(root);
//     }
//
//     void Deserialize(const std::string &str) {
//         Json::Value root;
//         if (Json::Reader reader; !reader.parse(str, root)) {
//             LOG_ERROR() << "parse error: " << reader.getFormattedErrorMessages();
//             return;
//         }
//         _result = root["result"].asInt();
//         _exitcode = root["exitcode"].asInt();
//     }
//
//     [[nodiscard]] const int &GetResult() const {
//         return _result;
//     }
//
//     [[nodiscard]] const int &GetExitCode() const {
//         return _exitcode;
//     }
//
//     void SetResult(const int &result) {
//         _result = result;
//     }
//
//     void SetExitCode(const int &exitcode) {
//         _exitcode = exitcode;
//     }
//
// private:
//     int _result;
//     int _exitcode;
// };

// template<class TaskType>
// class OnlineCalcProtoCol {
// private:
//     std::string sep = "\r\n";
//
// public:
//     explicit OnlineCalcProtoCol(const TaskType &task) : _task(task) {
//     }
//
//     [[nodiscard]] std::string Package(const std::string &json_str) const {
//         const size_t len = json_str.size();
//         return std::to_string(len) + sep + json_str + sep;
//     }
//
//     ssize_t Unpackage(std::string &stream_str, std::string &json_str_to_fill) const {
//         if (const auto pos = stream_str.find(sep); pos == std::string::npos) {
//             return 0;
//         } else {
//             const std::string pack_len_str = stream_str.substr(0, pos);
//             for (const auto &c: pack_len_str) {
//                 if (!isdigit(c)) {
//                     LOG_ERROR() << "invalid pack format: [package_len: " << pack_len_str << "]";
//                     return -1;
//                 }
//             }
//             const ssize_t pack_len = std::stoi(pack_len_str);
//             const ssize_t total_len = pack_len + pack_len_str.size() + sep.size() * 2;
//             if (stream_str.size() < total_len) {
//                 return 0;
//             }
//             json_str_to_fill = stream_str.substr(pos + sep.size(), pack_len);
//             stream_str.erase(0, total_len);
//             return pack_len;
//         }
//     }
//
//     [[nodiscard]] std::string HandleRequest(const std::string &stream_str) const {
//         std::string json_str;
//         // 解包
//         if (const ssize_t n = Unpackage(const_cast<std::string &>(stream_str), json_str); n <= 0) {
//             if (n == 0) {
//                 LOG_INFO() << "HandleRequest: stream_str is empty";
//             } else {
//                 LOG_WARN() << "unpackage failed or incomplete data";
//             }
//             return {};
//         }
//         // 反序列化
//         Request request;
//         request.Deserialize(json_str);
//
//         // 处理请求
//         const Response response = _task(request);
//
//         // 序列化
//         std::string result;
//         response.Serialize(result);
//         // 打包并发送
//         return Package(result);
//     }
// private:
//     TaskType _task;
// };

