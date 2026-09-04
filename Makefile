CC ?= gcc
VALGRIND ?= valgrind
CPPCHECK ?= cppcheck

COMMON_CFLAGS := \
	-std=c11 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wformat=2

STRICT_CFLAGS := \
	$(COMMON_CFLAGS) \
	-Werror

CFLAGS ?= $(COMMON_CFLAGS) -g -O0

CPPFLAGS_COMMON := \
	-D_POSIX_C_SOURCE=200809L

LDFLAGS ?=
LDLIBS ?=

VALGRIND_FLAGS := \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--track-fds=yes \
	--error-exitcode=99

CPPCHECK_FLAGS := \
	--std=c11 \
	--language=c \
	--enable=warning,performance,portability \
	--suppress=missingIncludeSystem \
	--error-exitcode=2

SRC_DIR := src
TEST_DIR := tests
INTEGRATION_DIR := $(TEST_DIR)/integration
COMMON_DIR := common
CONFIG_DIR := config

BUILD_DIR ?= build/default

TARGET := $(BUILD_DIR)/minishell
RUN_TARGET := shell
TEST_TARGET := $(BUILD_DIR)/minishell_tests
INTEGRATION_TARGET := $(BUILD_DIR)/minishell_integration_tests

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
	$(TEST_DIR)/test_job_control.c \
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

APP_CPPFLAGS := \
	$(CPPFLAGS_COMMON) \
	-Iinclude \
	-I$(COMMON_DIR) \
	-I$(CONFIG_DIR)

TEST_CPPFLAGS := \
	$(APP_CPPFLAGS) \
	-I$(TEST_DIR)

APP_OBJ := $(APP_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_OBJ := $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)
INTEGRATION_OBJ := $(INTEGRATION_SRC:%.c=$(BUILD_DIR)/%.o)

DEPS := \
	$(APP_OBJ:.o=.d) \
	$(TEST_OBJ:.o=.d) \
	$(INTEGRATION_OBJ:.o=.d)

.PHONY: \
	all \
	build \
	shell \
	run \
	test \
	integration \
	check \
	debug \
	release \
	asan \
	valgrind \
	strict \
	cppcheck \
	static \
	clean \
	help

all: shell

build: all

$(TARGET): $(APP_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

shell: $(TARGET)
	@echo "  COPY    $(RUN_TARGET)"
	@cp $(TARGET) $(RUN_TARGET)

run: shell
	./$(RUN_TARGET)

$(TEST_TARGET): $(TEST_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(INTEGRATION_TARGET): $(INTEGRATION_OBJ)
	@mkdir -p $(dir $@)
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	$(CC) $(APP_CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(INTEGRATION_DIR)/%.o: $(INTEGRATION_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

test: $(TEST_TARGET)
	@echo "  TEST    $(TEST_TARGET)"
	./$(TEST_TARGET)

integration: $(TARGET) $(INTEGRATION_TARGET)
	@echo "  TEST    $(INTEGRATION_TARGET)"
	MINISHELL_TEST_BIN="./$(TARGET)" \
	MINISHELL_TEST_DIR="./$(BUILD_DIR)" \
	./$(INTEGRATION_TARGET)

check: test integration shell

debug:
	@$(MAKE) \
		BUILD_DIR=build/debug \
		CFLAGS='$(COMMON_CFLAGS) -g -O0' \
		all

release:
	@$(MAKE) \
		BUILD_DIR=build/release \
		CFLAGS='$(COMMON_CFLAGS) -O2 -DNDEBUG' \
		all

asan:
	@ASAN_OPTIONS='detect_leaks=1:halt_on_error=1' \
	UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1' \
	$(MAKE) \
		BUILD_DIR=build/asan \
		CFLAGS='$(COMMON_CFLAGS) -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined' \
		LDFLAGS='-fsanitize=address,undefined' \
		check

valgrind: $(TARGET)
	@command -v $(VALGRIND) >/dev/null 2>&1 || { \
		echo "valgrind not found"; \
		exit 1; \
	}
	@echo "  VALGRIND basic"
	@printf 'echo hello\npwd\ntrue\nfalse\nstatus\nexit\n' | \
		$(VALGRIND) $(VALGRIND_FLAGS) ./$(TARGET)
	@echo "  VALGRIND redirect/pipeline"
	@printf 'echo hello > /tmp/minishell_valgrind.txt\ncat < /tmp/minishell_valgrind.txt\nprintf abc | wc -c\nsleep 1 | false\nstatus\nexit\n' | \
		$(VALGRIND) $(VALGRIND_FLAGS) ./$(TARGET)
	@rm -f /tmp/minishell_valgrind.txt
	@echo "  VALGRIND background shutdown"
	@printf 'sleep 30 &\nsleep 31 &\njobs\nexit\n' | \
		$(VALGRIND) $(VALGRIND_FLAGS) ./$(TARGET)
	@echo "  VALGRIND child stress"
	@{ \
		for i in $$(seq 1 100); do \
			echo "true &"; \
		done; \
		echo "sleep 1"; \
		echo "jobs"; \
		echo "exit"; \
	} | $(VALGRIND) $(VALGRIND_FLAGS) ./$(TARGET)

strict:
	@$(MAKE) \
		BUILD_DIR=build/strict \
		CFLAGS='$(STRICT_CFLAGS) -g -O0' \
		check

cppcheck:
	@command -v $(CPPCHECK) >/dev/null 2>&1 || { \
		echo "cppcheck not found"; \
		exit 1; \
	}
	@echo "  CPPCHECK"
	$(CPPCHECK) \
		$(CPPCHECK_FLAGS) \
		-D_POSIX_C_SOURCE=200809L \
		-Iinclude \
		-I$(COMMON_DIR) \
		-I$(CONFIG_DIR) \
		-I$(TEST_DIR) \
		$(SRC_DIR) \
		$(COMMON_DIR) \
		$(CONFIG_DIR) \
		$(TEST_DIR)

static: strict cppcheck

clean:
	@echo "  CLEAN"
	rm -rf build
	rm -f $(RUN_TARGET)

help:
	@echo "MiniShell build system"
	@echo ""
	@echo "  make                 Build MiniShell and create ./shell"
	@echo "  make build           Build MiniShell and create ./shell"
	@echo "  make run             Build and run ./shell"
	@echo "  make test            Build and run unit tests"
	@echo "  make integration     Build and run integration tests"
	@echo "  make check           Run unit + integration tests"
	@echo "  make debug           Build debug configuration"
	@echo "  make release         Build release configuration"
	@echo "  make asan            Run tests with ASan + LSan + UBSan"
	@echo "  make valgrind        Run runtime memory/fd checks"
	@echo "  make strict          Run full tests with compiler warnings as errors"
	@echo "  make cppcheck        Run cppcheck static analysis"
	@echo "  make static          Run strict build + cppcheck"
	@echo "  make clean           Remove all build products"
	@echo "  make help            Show this help"

-include $(DEPS)
