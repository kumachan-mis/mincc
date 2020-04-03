TARGET    := mincc.out
SRC_DIR   := ./src
TEST_DIR  := ./test
BUILD_DIR := ./build
SRCS      := $(shell find $(SRC_DIR) -name *.c)
OBJS      := $(SRCS:$(SRC_DIR)%.c=$(BUILD_DIR)%.o)
DEPS      := $(OBJS:%.o=%.dc)

CC        := gcc-9
CFLAGS    := -std=c99 -O2 -Wall -MMD -MP

MAKEDIR_P := mkdir -p


# src
$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(MAKEDIR_P) $(shell dirname $@) && $(CC) $(CFLAGS) -c $< -o $@ -MF $(BUILD_DIR)/$*.dc


# tests
test: $(BUILD_DIR)/test_vector.out $(BUILD_DIR)/test_map.out

$(BUILD_DIR)/test_vector.out:\
	$(BUILD_DIR)/test_vector.o $(BUILD_DIR)/common/vector.o $(BUILD_DIR)/common/memory.o
	$(CC) $^ -o $@

$(BUILD_DIR)/test_map.out:\
	$(BUILD_DIR)/test_map.o $(BUILD_DIR)/common/map.o $(BUILD_DIR)/common/memory.o
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	$(MAKEDIR_P) $(shell dirname $@) && $(CC) $(CFLAGS) -c $< -o $@ -MF $(BUILD_DIR)/$*.dc


# clean
clean:
	$(RM) -r $(BUILD_DIR)


.PHONY: test clean
-include $(DEPS)
