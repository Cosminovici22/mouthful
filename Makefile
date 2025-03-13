CFLAGS := ${CFLAGS} -march=native -pipe -O2 -Wall

BUILD_DIR := build
SRC_DIR := src
TESTS_DIR := tests

MFAPI_SRCS := mfapi.c
MFGEN_SRCS := array.c encoding.c mfgen.c mflexer_gen.c
MFTOK_SRCS := $(MFAPI_SRCS) mftok.c

MFAPI_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(MFAPI_SRCS))
MFGEN_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(MFGEN_SRCS))
MFTOK_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(MFTOK_SRCS))

.PHONY: all mfapi mfgen mftok test clean

all: mftok mfgen

mfapi: $(MFAPI_OBJS)

mfgen: $(MFGEN_OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $^

mftok: $(MFTOK_OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir $@

test: all
	$(TESTS_DIR)/test.sh

clean:
	$(RM) -r $(BUILD_DIR)
