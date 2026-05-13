#pragma once
#include <netinet/in.h>
#include <memory>

#include "../utils/InetAddr.hpp"

static constexpr int default_a = 32;

// class Socket {
// public:
//     enum SocketStatus {
//         NORMAL,
//         CREATE_ERROR,
//         BIND_ERROR,
//         LISTEN_ERROR,
//     };
//
//     virtual void CreateSocket() = 0;
//
//     virtual void BindSocket(in_port_t) = 0;
//
//     virtual void ListenSocket() = 0;
//
//     virtual void Close() = 0;
//
//     virtual std::shared_ptr<Socket> Acceptor(InetAddr &) = 0;
//
//     virtual bool Connect(const InetAddr &) = 0;
//
//     virtual ssize_t Recv(std::string &) = 0;
//
//     virtual bool Send(const std::string &) = 0;
//
//     virtual bool RecvFrom(std::string&, InetAddr&) = 0;
//
//     virtual const int &GetSockfd() = 0;
//
//     virtual ~Socket() = default;
//
// public:
//     void BuildTcpServerSocketMethod(const in_port_t port) {
//         CreateSocket();
//         BindSocket(port);
//         ListenSocket();
//     }
//
//     void BuildTcpClientSocketMethod() {
//         CreateSocket();
//     }
//
//     void BuildUdpServerSocketMethod(const in_port_t port) {
//         CreateSocket();
//         BindSocket(port);
//     }
// };

class Socket {
public:
    enum SocketStatus {
        NORMAL,
        CREATE_ERROR,
        BIND_ERROR,
        LISTEN_ERROR,
    };

    virtual void CreateSocket() = 0;

    virtual void BindSocket(in_port_t) = 0;

    virtual void Close() = 0;

    virtual const int &GetSockfd() = 0;

    virtual ~Socket() = default;
protected:
    int _sockfd = -1;
};

// TCP 接口
class ITcpSocket : public Socket {
public:
    virtual void ListenSocket() = 0;

    virtual std::shared_ptr<ITcpSocket> Acceptor(InetAddr &) = 0;

    virtual bool Connect(const InetAddr &) = 0;

    virtual ssize_t Recv(std::string &) = 0;

    virtual ssize_t Recv(std::string &, size_t) = 0;

    virtual bool Send(const std::string &) = 0;

public:
    void BuildServerSocketMethod(const in_port_t port) {
        CreateSocket();
        BindSocket(port);
        ListenSocket();
    }

    void BuildClientSocketMethod() {
        CreateSocket();
    }
};

class IUdpSocket : public Socket {
public:
    virtual bool RecvFrom(std::string &, InetAddr &) = 0;

    virtual bool SendTo(const std::string &, const InetAddr &) = 0;

public:
    void BuildServerSocketMethod(const in_port_t port) {
        CreateSocket();
        BindSocket(port);
    }

    void BuildClientSocketMethod() {
        CreateSocket();
    }
};
