define LINUX
endef

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude -Isrc -O2 -pthread
MAIN := UdpEchoServer.cpp
EXEC := server

OS := LINUX
ifdef WIN
	OS := WIN
endif

vpath %.h include
vpath %.cpp src

src := $(MAIN) $(wildcard src/*.cpp)
objs := $(patsubst %.cpp,build/%.o,$(notdir $(src)))

$(EXEC): $(objs)
	@echo OS=$(OS)
	$(CXX) -o $(EXEC) $(objs)

build/%.o: %.cpp
	@mkdir -p build
	$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
ifdef WIN
	del build\*.o $(EXEC).exe
else
	rm -rf build $(EXEC)
endif
.PHONY: clean
