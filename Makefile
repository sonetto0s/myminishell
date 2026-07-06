CC = gcc
CFLAGS = -Wall -Iinclude

SRC = src/main.c \
	  src/shell.c \
	  src/utils.c  \
	  src/log.c     \
	  src/parser.c
OBJ = $(SRC:.c=.o)

TARGET = minishell

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)