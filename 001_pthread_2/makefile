
OUT_DIT=bin
OUT=test
OUT_WITH_PATH=$(OUT_DIT)/$(OUT)


$(OUT):src/Main.cpp
	g++ -o $(OUT_WITH_PATH) $< -lpthread

clean:
	rm -f $(OUT_WITH_PATH)

run:$(OUT)
	$(OUT_WITH_PATH)

.PHONY: clean
