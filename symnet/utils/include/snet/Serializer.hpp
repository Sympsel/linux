#pragma once

#include <string>

/**
 * @brief 序列化与反序列化接口, 需要继承实现
 * @tparam RequestDataType 请求数据类型
 * @tparam ResponseDataType 响应数据类型
 */
template <class RequestDataType, class ResponseDataType>
class ISerializer {
public:
    using RequestType = RequestDataType;
    using ResponseType = ResponseDataType;
    virtual ~ISerializer() = default;

    // 请求序列化/反序列化
    virtual std::string SerializeRequest(const RequestDataType& req) const = 0;
    virtual bool DeserializeRequest(const std::string& str, RequestDataType& out) const = 0;

    // 响应序列化/反序列化
    virtual std::string SerializeResponse(const ResponseDataType& resp) const = 0;
    virtual bool DeserializeResponse(const std::string& str, ResponseDataType& out) const = 0;
};