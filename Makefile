CC = gcc
CFLAGS = -Wall -Iinclude -g

SRC = src/main.c \
	  src/shell.c \
	  src/utils.c  \
	  src/log.c     \
	  src/parser.c   \
	  src/executor.c  \
	  src/dispatcher.c \
	  src/builtin.c
OBJ = $(SRC:.c=.o)

TARGET = minishell

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)