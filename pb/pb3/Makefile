write: wirte.cc contacts.pb.cc
	g++ -o $@ $^ -std=c++20 $$(pkg-config --cflags --libs protobuf)
.PHPNY: clean

clean:
	rm -rf write