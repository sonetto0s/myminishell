# MiniShell V1.5 - GNU Make build system
#
# Responsibilities:
#   - Build the application and test executable.
#   - Keep application/test build products separate.
#   - Provide reproducible Debug/Release/ASan+UBSan targets.
#   - Keep POSIX feature requirements explicit.
#   - Generate dependency files automatically.
#
# CMake will become the primary build system at the end of V1.5.
# This Makefile remains a clean developer-facing entry point during V1.5.

# -----------------------------------------------------------------------------
# Toolchain / configuration
# -----------------------------------------------------------------------------

CC       ?= gcc
CFLAGS   ?= -std=c11 -Wall -Wextra -Wpedantic -Wformat=2 -g -O0
CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L -Iinclude -Icommon -Iconfig
LDFLAGS  ?=
LDLIBS   ?=

# Build products live outside the source tree.
BUILD_DIR ?= build/default

TARGET      := $(BUILD_DIR)/minishell
TEST_TARGET := $(BUILD_DIR)/minishell_tests

SRC_DIR  := src
TEST_DIR := tests
COMMON_DIR := common
CONFIG_DIR := config

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------

APP_SRC := \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/shell.c \
	$(SRC_DIR)/parser.c \
	$(SRC_DIR)/executor.c \
	$(SRC_DIR)/dispatcher.c \
	$(SRC_DIR)/builtin.c \
	$(SRC_DIR)/command.c \
	$(SRC_DIR)/sig.c \
	$(SRC_DIR)/shell_context.c \
	$(SRC_DIR)/job.c \
	$(SRC_DIR)/event.c \
	$(SRC_DIR)/builtin_table.c \
	$(SRC_DIR)/system_info.c \
	$(SRC_DIR)/terminal.c \
	$(COMMON_DIR)/utils.c \
	$(COMMON_DIR)/log.c \
	$(COMMON_DIR)/error.c \
	$(CONFIG_DIR)/config.c

# The test program has its own main(), so application main.c is intentionally
# excluded. Keep all implementation modules needed by the test target here.
TEST_SRC := \
	$(TEST_DIR)/test_main.c \
	$(TEST_DIR)/test_framework.c \
	$(TEST_DIR)/test_parser.c \
	$(TEST_DIR)/test_log.c \
	$(TEST_DIR)/test_config.c \
	$(SRC_DIR)/shell.c \
	$(SRC_DIR)/parser.c \
	$(SRC_DIR)/executor.c \
	$(SRC_DIR)/dispatcher.c \
	$(SRC_DIR)/builtin.c \
	$(TEST_DIR)/test_event.c \
	$(TEST_DIR)/test_shell_context.c  \
	$(SRC_DIR)/builtin_table.c \
	$(SRC_DIR)/command.c \
	$(TEST_DIR)/test_builtin_table.c \
	$(TEST_DIR)/test_system_info.c \
	$(TEST_DIR)/test_dispatcher.c \
	$(SRC_DIR)/sig.c \
	$(SRC_DIR)/shell_context.c \
	$(SRC_DIR)/job.c \
	$(TEST_DIR)/test_job.c \
	$(SRC_DIR)/event.c \
	$(SRC_DIR)/system_info.c \
	$(SRC_DIR)/terminal.c \
	$(COMMON_DIR)/utils.c \
	$(COMMON_DIR)/log.c \
	$(COMMON_DIR)/error.c \
	$(CONFIG_DIR)/config.c \
	$(TEST_DIR)/test_command.c

APP_OBJ  := $(APP_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_OBJ := $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)
DEPS     := $(APP_OBJ:.o=.d) $(TEST_OBJ:.o=.d)

# -----------------------------------------------------------------------------
# Phony targets
# -----------------------------------------------------------------------------

.PHONY: all build test check debug release asan valgrind clean help

# -----------------------------------------------------------------------------
# Default target
# -----------------------------------------------------------------------------

all: $(TARGET)

build: all

# -----------------------------------------------------------------------------
# Link targets
# -----------------------------------------------------------------------------

$(TARGET): $(APP_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TEST_TARGET): $(TEST_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# -----------------------------------------------------------------------------
# Compile rule
# -----------------------------------------------------------------------------

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# -----------------------------------------------------------------------------
# Test / quality targets
# -----------------------------------------------------------------------------

# Build the test executable and run it. A non-zero test exit status propagates
# to make, which is required for later CI integration.
test: $(TEST_TARGET)
	@echo "  TEST    $(TEST_TARGET)"
	./$(TEST_TARGET)

# Current check target is intentionally small. It will become the single
# quality-gate entry point as V1.5 testing/static-analysis grows.
check: test

# -----------------------------------------------------------------------------
# Debug / Release / Sanitizer targets
# -----------------------------------------------------------------------------

# Keep different configurations in different build trees so object files from
# one configuration can never silently contaminate another.
debug:
	@$(MAKE) BUILD_DIR=build/debug \
		CFLAGS='-std=c11 -Wall -Wextra -Wpedantic -Wformat=2 -g -O0' \
		all

release:
	@$(MAKE) BUILD_DIR=build/release \
		CFLAGS='-std=c11 -Wall -Wextra -Wpedantic -Wformat=2 -O2 -DNDEBUG' \
		all

asan:
	@$(MAKE) BUILD_DIR=build/asan \
		CFLAGS='-std=c11 -Wall -Wextra -Wpedantic -Wformat=2 -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined' \
		LDFLAGS='-fsanitize=address,undefined' \
		all

valgrind:
	@$(MAKE) BUILD_DIR=build/debug debug
	@$(MAKE) BUILD_DIR=build/debug test
	@echo "  VALGRIND $(TEST_TARGET)"
	valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes $(TEST_TARGET)

# -----------------------------------------------------------------------------
# Cleanup
# -----------------------------------------------------------------------------

clean:
	@echo "  CLEAN"
	rm -rf build

# -----------------------------------------------------------------------------
# Help
# -----------------------------------------------------------------------------

help:
	@echo "MiniShell build system"
	@echo ""
	@echo "  make            Build MiniShell (default configuration)"
	@echo "  make test       Build and run automated tests"
	@echo "  make check      Run the current quality gate"
	@echo "  make debug      Build Debug configuration"
	@echo "  make release    Build Release configuration"
	@echo "  make asan       Build with ASan + UBSan"
	@echo "  make valgrind   Run tests under Valgrind"
	@echo "  make clean      Remove all build products"
	@echo "  make help       Show this help"

# -----------------------------------------------------------------------------
# Auto-generated dependency files
# -----------------------------------------------------------------------------

-include $(DEPS)
