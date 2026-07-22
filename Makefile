CC = gcc
CFLAGS = -Wall -g
CFLAGS += -I./include -I./common

SRC = src/main.c   \
      src/shell.c   \
      common/utils.c \
      common/log.c    \
      src/parser.c 	   \
      src/executor.c    \
      src/dispatcher.c   \
      src/builtin.c	      \
	  src/command.c	       

OBJ = $(patsubst %.c, build/%.o, $(SRC))

TARGET = shell

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)