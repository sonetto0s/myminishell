CC := gcc

TARGET := shell
TEST_TARGET := test


BUILD_DIR := build


SRC_DIR := src
TEST_DIR := tests


CFLAGS := -Wall -Wextra -g

CFLAGS += -Iinclude
CFLAGS += -Icommon
CFLAGS += -Iconfig

CFLAGS += -MMD -MP


LDFLAGS :=


APP_SRC := \
	src/main.c \
	src/shell.c \
	src/parser.c \
	src/executor.c \
	src/dispatcher.c \
	src/builtin.c \
	src/command.c \
	src/sig.c \
	src/shell_context.c \
	src/job.c \
	src/event.c \
	src/builtin_table.c \
	common/utils.c \
	common/log.c \
	common/error.c \
	config/config.c \
	src/system_info.c \
	src/terminal.c



CORE_SRC := \
	src/shell.c \
	src/parser.c \
	src/executor.c \
	src/dispatcher.c \
	src/builtin.c \
	src/command.c \
	src/sig.c \
	src/shell_context.c \
	src/builtin_table.c \
	src/job.c \
	src/event.c \
	src/terminal.c \
	common/utils.c \
	common/log.c \
	common/error.c \
	config/config.c


TEST_SRC := \
	tests/test_main.c \
	tests/test_parser.c \
	tests/test_log.c \
	tests/test_config.c \
	$(CORE_SRC)

APP_OBJ := $(APP_SRC:%.c=$(BUILD_DIR)/%.o)

TEST_OBJ := $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)


DEP := \
	$(APP_OBJ:.o=.d) \
	$(TEST_OBJ:.o=.d)




.PHONY: all test check debug release clean help


all: $(TARGET)




$(TARGET): $(APP_OBJ)

	@echo "LD $@"

	$(CC) $^ $(LDFLAGS) -o $@

$(TEST_TARGET): $(TEST_OBJ)

	@echo "LD $@"

	$(CC) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.c

	@mkdir -p $(dir $@)

	@echo "CC $<"

	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)


check: $(TEST_TARGET)

	@echo "Running tests..."

	./$(TEST_TARGET)


debug:

	$(MAKE) clean

	$(MAKE) \
	CFLAGS="$(CFLAGS) -fsanitize=address" \
	LDFLAGS="-fsanitize=address"


release:

	$(MAKE) clean

	$(MAKE) \
	CFLAGS="-Wall -Wextra -O2 $(CFLAGS)"


clean:

	rm -rf $(BUILD_DIR)

	rm -f $(TARGET)

	rm -f $(TEST_TARGET)

-include $(DEP)


help:

	@echo ""
	@echo "MiniShell Build System"
	@echo ""
	@echo "make          Build MiniShell"
	@echo "make test     Build test program"
	@echo "make check    Run tests"
	@echo "make debug    Build with AddressSanitizer"
	@echo "make release  Build optimized version"
	@echo "make clean    Remove build files"
	@echo ""
