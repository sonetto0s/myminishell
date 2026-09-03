# ==============================================================================
# MiniShell V1.5 - GNU Make Build System
# ==============================================================================
#
# Responsibilities:
#   - Build the MiniShell application.
#   - Build and run unit tests.
#   - Build and run integration tests against the real MiniShell binary.
#   - Keep application / unit-test / integration-test products separated.
#   - Provide Debug / Release / ASan+UBSan / Valgrind targets.
#   - Generate dependency files automatically.
#   - Keep POSIX feature requirements explicit.
#
# CMake will become the primary build system at the end of V1.5.
# This Makefile remains a clean developer-facing entry point during V1.5.
#
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

SRC_DIR       := src
TEST_DIR      := tests
INTEGRATION_DIR := $(TEST_DIR)/integration
COMMON_DIR    := common
CONFIG_DIR    := config

BUILD_DIR ?= build/default


# ------------------------------------------------------------------------------
# Output targets
# ------------------------------------------------------------------------------

TARGET             := $(BUILD_DIR)/minishell
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
#
# Unit tests have their own main().
# Therefore application main.c is intentionally excluded.
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
#
# IMPORTANT:
# These tests do NOT link against MiniShell implementation modules.
# They launch $(TARGET) as a separate process.
#
# Add a source file here only after the file actually exists.
# ------------------------------------------------------------------------------

INTEGRATION_SRC := \
	$(INTEGRATION_DIR)/test_integration_main.c \
	$(INTEGRATION_DIR)/test_shell_runner.c \
	$(INTEGRATION_DIR)/test_shell_basic.c \
	$(INTEGRATION_DIR)/test_shell_redirect.c \
	$(INTEGRATION_DIR)/test_shell_pipeline.c \
	$(INTEGRATION_DIR)/test_shell_status.c \
	$(INTEGRATION_DIR)/test_shell_background.c \
	$(TEST_DIR)/test_framework.c

# ------------------------------------------------------------------------------
# Per-target preprocessor flags
#
# Production code:
#   include/, common/, config/
#
# Unit / integration tests:
#   additionally need tests/
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
	test \
	integration \
	check \
	debug \
	release \
	asan \
	valgrind \
	clean \
	help


# ------------------------------------------------------------------------------
# Default target
# ------------------------------------------------------------------------------

all: $(TARGET)

build: all


# ------------------------------------------------------------------------------
# Application
# ------------------------------------------------------------------------------

$(TARGET): $(APP_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


# ------------------------------------------------------------------------------
# Unit-test executable
# ------------------------------------------------------------------------------

$(TEST_TARGET): $(TEST_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


# ------------------------------------------------------------------------------
# Integration-test executable
#
# This executable contains only the integration-test runner.
# MiniShell itself is built separately as $(TARGET).
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
# Compile unit/integration test objects
#
# Tests need -Itests in addition to the normal project include paths.
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
#
# First build the real MiniShell.
# Then build the integration-test runner.
# Then run the runner.
# ------------------------------------------------------------------------------

integration: $(TARGET) $(INTEGRATION_TARGET)
	@echo "  TEST    $(INTEGRATION_TARGET)"
	./$(INTEGRATION_TARGET)


# ------------------------------------------------------------------------------
# Quality gate
#
# This is the command CI will eventually call.
# ------------------------------------------------------------------------------

check: test integration


# ------------------------------------------------------------------------------
# Debug
# ------------------------------------------------------------------------------

debug:
	@$(MAKE) \
		BUILD_DIR=build/debug \
		CFLAGS='$(COMMON_CFLAGS) -g -O0' \
		all


# ------------------------------------------------------------------------------
# Release
# ------------------------------------------------------------------------------

release:
	@$(MAKE) \
		BUILD_DIR=build/release \
		CFLAGS='$(COMMON_CFLAGS) -O2 -DNDEBUG' \
		all


# ------------------------------------------------------------------------------
# ASan + UBSan
#
# Builds a separate sanitizer tree so normal objects can never contaminate it.
# ------------------------------------------------------------------------------

asan:
	@$(MAKE) \
		BUILD_DIR=build/asan \
		CFLAGS='$(COMMON_CFLAGS) -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined' \
		LDFLAGS='-fsanitize=address,undefined' \
		all


# ------------------------------------------------------------------------------
# Valgrind
#
# Build debug unit-test executable, then run it under Valgrind.
# ------------------------------------------------------------------------------

valgrind:
	@$(MAKE) \
		BUILD_DIR=build/debug \
		CFLAGS='$(COMMON_CFLAGS) -g -O0' \
		test
	@echo "  VALGRIND $(BUILD_DIR)/minishell_tests"
	valgrind \
		--leak-check=full \
		--show-leak-kinds=all \
		--track-fds=yes \
		$(BUILD_DIR)/minishell_tests


# ------------------------------------------------------------------------------
# Cleanup
# ------------------------------------------------------------------------------

clean:
	@echo "  CLEAN"
	rm -rf build


# ------------------------------------------------------------------------------
# Help
# ------------------------------------------------------------------------------

help:
	@echo "MiniShell build system"
	@echo ""
	@echo "  make                 Build MiniShell"
	@echo "  make build           Build MiniShell"
	@echo "  make test            Build and run unit tests"
	@echo "  make integration     Build and run integration tests"
	@echo "  make check           Run unit + integration tests"
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
