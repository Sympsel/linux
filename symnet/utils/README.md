# Utils 工具库

SymNet 项目的核心工具库，提供网络编程、日志记录、线程管理和配置解析等基础功能模块。

## 📁 目录结构

```txt
utils/include/
├── module/ # 模块聚合头文件（统一导出接口）
├── network/ # 网络通信模块
├── slog/ # 日志系统模块
├── thread/ # 线程与并发模块
└── utils/ # 通用工具类
```

## 📦 模块说明

### 1. Module 模块聚合层 (`module/`)

提供统一的模块入口，简化外部调用。

- **SymINet.h** - 网络模块聚合头文件
  - 统一导出 TCP/UDP Socket 相关接口
  - 包含：`TcpCSocket`, `TcpSSocket`, `UdpSocket`
  
- **SymLog.h** - 日志模块聚合头文件
  - 统一导出日志系统接口
  - 支持控制台、文件和组合输出模式
  
- **SymThread.h** - 线程模块聚合头文件
  - 统一导出线程管理接口
  - 包含：线程、互斥锁、条件变量、线程池

### 2. Network 网络模块 (`network/`)

完整的 TCP/UDP 网络通信组件。

- **SocketUtils.h** - Socket 工具函数
  - 提供底层的 Socket 创建、读写操作
  - 封装系统调用，简化错误处理
  
- **TcpSocket.hpp** - TCP Socket 基类
  - 管理 Socket 生命周期（创建、销毁）
  - 提供 Connect、Listen、Accept 等静态方法
  
- **TcpCSocket.hpp** - TCP 客户端 Socket
  - 封装客户端连接逻辑
  - 自动处理连接建立和断开
  
- **TcpSSocket.hpp** - TCP 服务端 Socket
  - 封装服务端绑定、监听、接受连接
  - 支持 SO_REUSEADDR 端口重用
  
- **UdpSocket.hpp** - UDP Socket
  - 无连接的 UDP 通信
  - 支持广播和多播

### 3. Slog 日志模块 (`slog/`)

灵活的多策略日志系统。

- **Log.h** - 日志系统核心
  - 多级别日志：DEBUG, INFO, WARNING, ERROR, FATAL
  - 多输出策略：
    - `ConsoleLogStrategy` - 控制台输出（支持彩色）
    - `FileLogStrategy` - 文件输出（支持按日轮转）
    - `CombinedLogStrategy` - 控制台 + 文件组合输出
  - 线程安全的日志写入
  - 便捷的宏定义：`LOG_DEBUG()`, `LOG_INFO()`, `LOG_ERROR()` 等
  - ANSI 颜色码支持，便于终端调试

### 4. Thread 线程模块 (`thread/`)

完整的并发编程组件。

- **Thread.hpp** - 线程封装类
  - RAII 风格的线程管理
  - 支持启动、等待、分离等操作
  
- **Mutex.hpp** - 互斥锁封装
  - 基于 RAII 的锁管理
  - 防止死锁和资源泄漏
  
- **Cond.hpp** - 条件变量封装
  - 线程间同步机制
  - 支持等待和通知操作
  
- **ThreadPool.hpp** - 线程池实现
  - 固定大小的工作线程池
  - 任务队列管理
  - 自动负载均衡

### 5. Utils 通用工具 (`utils/`)

常用辅助工具类。

- **Conf.hpp** - 配置管理中心
  - 集中管理项目配置常量
  - 网络默认参数（IP、端口、缓冲区大小等）
  - 线程池默认配置
  - 使用 inline static 避免多重定义
  
- **InetAddr.hpp** - 网络地址封装
  - 封装 `sockaddr_in` 结构
  - 提供 IP 地址和端口的便捷操作
  - 支持字符串和数值格式的转换
  
- **IniParse.hpp** - INI 配置文件解析器
  - 解析标准 INI 格式配置文件
  - 支持节（section）和键值对
  - 提供类型安全的值读取

## 🚀 快速开始

### 使用网络模块

```c++
#include "module/SymINet.h"
// TCP 客户端
TcpCSocket client;
client.Connect(InetAddr(8080, "127.0.0.1"));
// TCP 服务端
TcpSSocket server(8080);
server.Bind();
server.Listen();
```

### 使用日志系统
```c++
#include "module/SymLog.h"
// 配置日志策略
USE_COMBINED_LOG("./log/", "app.log", true, true);
// 记录日志
LOG_DEBUG() << "Debug message";
LOG_INFO() << "Info message";
LOG_ERROR() << "Error: " << strerror(errno);
```

### 使用线程模块

```c++
#include "module/SymThread.h"
ThreadPool pool(4); // 线程数
pool.Start();
pool.AddTask({
    // 任务代码 
});
```

### 使用配置管理

```c++
#include "utils/Conf.hpp"
// 访问配置常量
auto port = Conf::network_default_port;
auto backlog = Conf::network_backlog;
```

## 📝 注意事项

1. **头文件包含**：推荐使用 `module/` 下的聚合头文件，而非直接包含底层实现
2. **日志策略**：建议在程序启动时配置日志策略，避免运行时频繁切换
3. **线程池**：确保在程序退出前正确停止线程池
4. **Socket 生命周期**：所有 Socket 类都采用 RAII 管理，无需手动关闭

## 📄 other
