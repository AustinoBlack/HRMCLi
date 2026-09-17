CC := gcc

CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Iinclude

TARGET := hrmcli

SRC := \
	src/main.c \
	src/app.c \
	src/config.c \
	src/log.c \
	src/terminal.c\
	src/tui.c

OBJ := $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
