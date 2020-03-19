TARGET    := mincc.out
SRC_DIR   := ./src
TEST_DIR  := ./test
BUILD_DIR := ./build
SRCS      := $(shell find $(SRC_DIR) -name *.c)
OBJS      := $(SRCS:$(SRC_DIR)%.c=$(BUILD_DIR)%.o)

CC        := gcc-9
CFLAGS    := -std=c11 -g -static

MAKEDIR_P     := mkdir -p

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(MAKEDIR_P) $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@

test:
	TEST_DIR/test.sh

clean:
	$(RM) -r $(BUILD_DIR)

.PHONY: test clean
