CFLAGS := ${CFLAGS} -march=native -pipe -O2 -Wall
BUILD_DIR := ../../build/mfapi
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: mfapi mkdir_build clean

mfapi: $(OBJS)
	$(CC) $(CFLAGS) -shared -o $(BUILD_DIR)/$@ $^

$(BUILD_DIR)/%.o: %.c | mkdir_build
	$(CC) $(CFLAGS) -fpic -c -o $@ $<

mkdir_build:
	mkdir -p $(BUILD_DIR)

clean:
	$(RM) -r $(BUILD_DIR)
