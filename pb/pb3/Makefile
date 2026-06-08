all: write read

write: write.cc contacts.pb.cc
	g++ -o $@ $^ -std=c++20 $$(pkg-config --cflags --libs protobuf)

read: read.cc contacts.pb.cc
	g++ -o $@ $^ -std=c++20 $$(pkg-config --cflags --libs protobuf)
.PHPNY: clean

trunc:
	rm -rf contacts.bin

clean:
	rm -rf write
	rm -rf read