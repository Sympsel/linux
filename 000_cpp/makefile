test: test.cpp
	mkdir -p bin
	g++ -o bin/$@ -O2 -std=c++17 $<

clean: test
	rm -f bin/test

.PHONY: clean
