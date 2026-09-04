# ==============================================================================
# MiniShell Makefile
# ==============================================================================


# ------------------------------------------------------------------------------
# Toolchain
# ------------------------------------------------------------------------------

CC ?= gcc


# ------------------------------------------------------------------------------
# Common compile / link flags
# ------------------------------------------------------------------------------

COMMON_CFLAGS := \
	-std=c11 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wformat=2

CFLAGS ?= $(COMMON_CFLAGS) -g -O0

CPPFLAGS_COMMON := \
	-D_POSIX_C_SOURCE=200809L

LDFLAGS ?=
LDLIBS  ?=


# ------------------------------------------------------------------------------
# Directories
# ------------------------------------------------------------------------------

SRC_DIR         := src
TEST_DIR        := tests
INTEGRATION_DIR := $(TEST_DIR)/integration
COMMON_DIR      := common
CONFIG_DIR      := config

BUILD_DIR ?= build/default


# ------------------------------------------------------------------------------
# Output targets
# ------------------------------------------------------------------------------

TARGET             := $(BUILD_DIR)/minishell
RUN_TARGET         := shell
TEST_TARGET        := $(BUILD_DIR)/minishell_tests
INTEGRATION_TARGET := $(BUILD_DIR)/minishell_integration_tests


# ------------------------------------------------------------------------------
# Production sources
# ------------------------------------------------------------------------------

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


# ------------------------------------------------------------------------------
# Unit-test sources
# ------------------------------------------------------------------------------

TEST_SRC := \
	$(TEST_DIR)/test_main.c \
	$(TEST_DIR)/test_framework.c \
	$(TEST_DIR)/test_parser.c \
	$(TEST_DIR)/test_log.c \
	$(TEST_DIR)/test_config.c \
	$(TEST_DIR)/test_event.c \
	$(TEST_DIR)/test_shell_context.c \
	$(TEST_DIR)/test_builtin_table.c \
	$(TEST_DIR)/test_system_info.c \
	$(TEST_DIR)/test_dispatcher.c \
	$(TEST_DIR)/test_executor.c \
	$(TEST_DIR)/test_job.c \
	$(TEST_DIR)/test_command.c \
	$(SRC_DIR)/shell.c \
	$(SRC_DIR)/parser.c \
	$(SRC_DIR)/executor.c \
	$(SRC_DIR)/dispatcher.c \
	$(SRC_DIR)/builtin.c \
	$(SRC_DIR)/builtin_table.c \
	$(SRC_DIR)/command.c \
	$(SRC_DIR)/sig.c \
	$(SRC_DIR)/shell_context.c \
	$(SRC_DIR)/job.c \
	$(SRC_DIR)/event.c \
	$(SRC_DIR)/system_info.c \
	$(SRC_DIR)/terminal.c \
	$(COMMON_DIR)/utils.c \
	$(COMMON_DIR)/log.c \
	$(COMMON_DIR)/error.c \
	$(CONFIG_DIR)/config.c


# ------------------------------------------------------------------------------
# Integration-test sources
# ------------------------------------------------------------------------------

INTEGRATION_SRC := \
	$(INTEGRATION_DIR)/test_integration_main.c \
	$(INTEGRATION_DIR)/test_shell_runner.c \
	$(INTEGRATION_DIR)/test_shell_basic.c \
	$(INTEGRATION_DIR)/test_shell_redirect.c \
	$(INTEGRATION_DIR)/test_shell_pipeline.c \
	$(INTEGRATION_DIR)/test_shell_status.c \
	$(INTEGRATION_DIR)/test_shell_background.c \
	$(INTEGRATION_DIR)/test_shell_pty.c \
	$(TEST_DIR)/test_framework.c


# ------------------------------------------------------------------------------
# Preprocessor flags
# ------------------------------------------------------------------------------

APP_CPPFLAGS := \
	$(CPPFLAGS_COMMON) \
	-Iinclude \
	-I$(COMMON_DIR) \
	-I$(CONFIG_DIR)

TEST_CPPFLAGS := \
	$(APP_CPPFLAGS) \
	-I$(TEST_DIR)


# ------------------------------------------------------------------------------
# Object files
# ------------------------------------------------------------------------------

APP_OBJ := \
	$(APP_SRC:%.c=$(BUILD_DIR)/%.o)

TEST_OBJ := \
	$(TEST_SRC:%.c=$(BUILD_DIR)/%.o)

INTEGRATION_OBJ := \
	$(INTEGRATION_SRC:%.c=$(BUILD_DIR)/%.o)


# ------------------------------------------------------------------------------
# Dependency files
# ------------------------------------------------------------------------------

DEPS := \
	$(APP_OBJ:.o=.d) \
	$(TEST_OBJ:.o=.d) \
	$(INTEGRATION_OBJ:.o=.d)


# ------------------------------------------------------------------------------
# Phony targets
# ------------------------------------------------------------------------------

.PHONY: \
	all \
	build \
	run \
	test \
	integration \
	check \
	debug \
	release \
	asan \
	valgrind \
	clean \
	help \
	shell


# ------------------------------------------------------------------------------
# Default target
# ------------------------------------------------------------------------------

all: shell

build: all


# ------------------------------------------------------------------------------
# Application
# ------------------------------------------------------------------------------

$(TARGET): $(APP_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


# ------------------------------------------------------------------------------
# Root executable
# ------------------------------------------------------------------------------

shell: $(TARGET)
	@echo "  COPY    $(RUN_TARGET)"
	@cp $(TARGET) $(RUN_TARGET)


# ------------------------------------------------------------------------------
# Run application
# ------------------------------------------------------------------------------

run: shell
	./$(RUN_TARGET)


# ------------------------------------------------------------------------------
# Unit-test executable
# ------------------------------------------------------------------------------

$(TEST_TARGET): $(TEST_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


# ------------------------------------------------------------------------------
# Integration-test executable
# ------------------------------------------------------------------------------

$(INTEGRATION_TARGET): $(INTEGRATION_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


# ------------------------------------------------------------------------------
# Compile production objects
# ------------------------------------------------------------------------------

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	$(CC) $(APP_CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@


# ------------------------------------------------------------------------------
# Compile unit-test objects
# ------------------------------------------------------------------------------

$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@


# ------------------------------------------------------------------------------
# Compile integration-test objects
# ------------------------------------------------------------------------------

$(BUILD_DIR)/$(INTEGRATION_DIR)/%.o: $(INTEGRATION_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@


# ------------------------------------------------------------------------------
# Unit tests
# ------------------------------------------------------------------------------

test: $(TEST_TARGET)
	@echo "  TEST    $(TEST_TARGET)"
	./$(TEST_TARGET)


# ------------------------------------------------------------------------------
# Integration tests
# ------------------------------------------------------------------------------

integration: $(TARGET) $(INTEGRATION_TARGET)
	@echo "  TEST    $(INTEGRATION_TARGET)"
	./$(INTEGRATION_TARGET)


# ------------------------------------------------------------------------------
# Quality gate
# ------------------------------------------------------------------------------

check: test integration shell


# ------------------------------------------------------------------------------
# Debug build
# ------------------------------------------------------------------------------

debug:
	@$(MAKE) \
		BUILD_DIR=build/debug \
		CFLAGS='$(COMMON_CFLAGS) -g -O0' \
		all


# ------------------------------------------------------------------------------
# Release build
# ------------------------------------------------------------------------------

release:
	@$(MAKE) \
		BUILD_DIR=build/release \
		CFLAGS='$(COMMON_CFLAGS) -O2 -DNDEBUG' \
		all


# ------------------------------------------------------------------------------
# ASan + UBSan build
# ------------------------------------------------------------------------------

asan:
	@$(MAKE) \
		BUILD_DIR=build/asan \
		CFLAGS='$(COMMON_CFLAGS) -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined' \
		LDFLAGS='-fsanitize=address,undefined' \
		all


# ------------------------------------------------------------------------------
# Valgrind
# ------------------------------------------------------------------------------

valgrind:
	@$(MAKE) \
		BUILD_DIR=build/debug \
		CFLAGS='$(COMMON_CFLAGS) -g -O0' \
		test
	@echo "  VALGRIND build/debug/minishell_tests"
	valgrind \
		--leak-check=full \
		--show-leak-kinds=all \
		--track-fds=yes \
		build/debug/minishell_tests


# ------------------------------------------------------------------------------
# Cleanup
# ------------------------------------------------------------------------------

clean:
	@echo "  CLEAN"
	rm -rf build
	rm -f $(RUN_TARGET)


# ------------------------------------------------------------------------------
# Help
# ------------------------------------------------------------------------------

help:
	@echo "MiniShell build system"
	@echo ""
	@echo "  make                 Build MiniShell and create ./shell"
	@echo "  make build           Build MiniShell and create ./shell"
	@echo "  make run             Build and run ./shell"
	@echo "  make test            Build and run unit tests"
	@echo "  make integration     Build and run integration tests"
	@echo "  make check           Run unit + integration tests and create ./shell"
	@echo "  make debug           Build Debug configuration"
	@echo "  make release         Build Release configuration"
	@echo "  make asan            Build with ASan + UBSan"
	@echo "  make valgrind        Run unit tests under Valgrind"
	@echo "  make clean           Remove all build products"
	@echo "  make help            Show this help"


# ------------------------------------------------------------------------------
# Auto-generated dependency files
# ------------------------------------------------------------------------------

-include $(DEPS)
