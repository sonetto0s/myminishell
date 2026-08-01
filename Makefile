CC = gcc

CFLAGS = -Wall -g
CFLAGS += -I./include
CFLAGS += -I./common
CFLAGS += -I./config

TARGET = shell

SRC = src/main.c \
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
      common/utils.c \
      common/log.c \
      common/error.c \
      config/config.c


OBJ = $(patsubst %.c,build/%.o,$(SRC))

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)


TEST = test


TEST_SRC = tests/test_main.c \
           tests/test_config.c \
           tests/test_parser.c \
           tests/test_log.c \
           src/parser.c \
           src/command.c \
           src/shell_context.c \
           src/job.c \
           src/event.c \
           config/config.c \
           common/utils.c \
           common/log.c
		   
TEST_OBJ = $(patsubst %.c,build/%.o,$(TEST_SRC))


$(TEST): $(TEST_OBJ)
	$(CC) $(TEST_OBJ) -o $(TEST)


build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean test run


test: $(TEST)

run: $(TARGET)
	./$(TARGET)


clean:
	rm -rf build
	rm -f $(TARGET)
	rm -f $(TEST)