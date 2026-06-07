CXX = c++
CXXFLAGS = -g -std=c++20 -fdiagnostics-color=always -fmodules-ts -DW
LDFLAGS = -lncurses -ljsoncpp -lpthread -lmpg123

SRC = src/main.cpp
OUTDIR = out
TARGET = $(OUTDIR)/snake

SRCDIR = src
HEADERS = $(SRCDIR)/*.hpp $(SRCDIR)/thread/*.hpp

OBJ = $(OUTDIR)/main.o

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ) | $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ): $(SRC) $(HEADERS) | $(OUTDIR)
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTDIR):
	mkdir -p $(OUTDIR)

run: $(TARGET)
	cd $(shell pwd) && ./$(TARGET)

clean:
	rm -rf $(OUTDIR)
