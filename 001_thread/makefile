
OUT_DIT=bin
OUT=create_thread
OUT_WITH_PATH=$(OUT_DIT)/$(OUT)


$(OUT):src/test.cpp
	g++ -o $(OUT_WITH_PATH) $< -lpthread

clean:
	rm -f $(OUT_WITH_PATH)

run:$(OUT)
	$(OUT_WITH_PATH)

.PHONY: clean
