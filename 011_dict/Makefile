# 编译器设置
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Isrc -O2
# CXXFLAGS := -std=c++17 -Wall -Wextra -Isrc -O2 -pthread

# 目标
CLIENT := client
SERVER := server

# 源文件
CLIENT_SRC := Client.cpp
SERVER_SRC := Server.cpp

# 对象文件（放在 build/ 目录）
CLIENT_OBJ := build/Client.o
SERVER_OBJ := build/Server.o

# 默认编译两个
all: $(CLIENT) $(SERVER)

# 链接客户端
$(CLIENT): $(CLIENT_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

# 链接服务端
$(SERVER): $(SERVER_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

# 编译规则：根目录的 .cpp → build/ 的 .o
build/%.o: %.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 创建 build 目录
build:
	@mkdir -p build

# 清理
clean:
	rm -rf build $(CLIENT) $(SERVER)

.PHONY: all clean