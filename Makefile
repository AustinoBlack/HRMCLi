CC := gcc

CPPFLAGS := \
	-D_POSIX_C_SOURCE=200809L \
	-Iinclude

CFLAGS := \
	-std=c11 \
	-Wall \
	-Wextra \
	-Wpedantic

LDLIBS := -lcjson

TARGET := hrmcli

SRC := \
	src/main.c \
	src/app.c \
	src/config.c \
	src/log.c \
	src/node.c \
	src/node_config.c \
	src/terminal.c \
	src/ui.c \
	src/menu.c \
	src/pane.c \
	src/workspace.c \
	src/tui.c \
	src/screens/main_menu.c \
	src/screens/dashboard.c \
	src/screens/nodes.c \
	src/screens/configuration.c \
	src/screens/logs.c \
	src/screens/cli.c \
	src/screens/system.c \

OBJ := $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
