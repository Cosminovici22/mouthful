CFLAGS := ${CFLAGS} -march=native -pipe -O2 -Wall
BUILD_DIR := ../../build/mfgen
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: mfgen mkdir_build clean

mfgen: $(OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $^

$(BUILD_DIR)/%.o: %.c | mkdir_build
	$(CC) $(CFLAGS) -c -o $@ $<

mkdir_build:
	mkdir -p $(BUILD_DIR)

clean:
	$(RM) -r $(BUILD_DIR)
