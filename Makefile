TARGET    := mincc.out
SRC_DIR   := ./src
TEST_DIR  := ./test
BUILD_DIR := ./build
SRCS      := $(shell find $(SRC_DIR) -name *.c)
OBJS      := $(SRCS:$(SRC_DIR)%.c=$(BUILD_DIR)%.o)

CC        := gcc-9
CFLAGS    := -std=c99 -g -static

MAKEDIR_P     := mkdir -p


$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(MAKEDIR_P) $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@


$(BUILD_DIR)/test_vector.out:\
	$(BUILD_DIR)/test_vector.o $(BUILD_DIR)/vector.o $(BUILD_DIR)/memory.o
	$(CC) $^ -o $@

$(BUILD_DIR)/test_map.out:\
	$(BUILD_DIR)/test_map.o $(BUILD_DIR)/map.o $(BUILD_DIR)/memory.o
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	$(MAKEDIR_P) $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@


test:
	TEST_DIR/test.sh

clean:
	$(RM) -r $(BUILD_DIR)

.PHONY: test clean
