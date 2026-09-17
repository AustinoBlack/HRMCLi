#ifndef HRMCLI_TERMINAL_H
#define HRMCLI_TERMINAL_H

typedef struct {
    int rows;
    int cols;
} TerminalSize;

/*
 * Ordinary characters are returned as their byte value (0-255).
 * Special keys begin at 1000 to avoid collisions.
 */
typedef enum {
    TERMINAL_KEY_UP = 1000,
    TERMINAL_KEY_DOWN,
    TERMINAL_KEY_LEFT,
    TERMINAL_KEY_RIGHT,
    TERMINAL_KEY_ENTER,
    TERMINAL_KEY_ESCAPE,
    TERMINAL_KEY_BACKSPACE,
    TERMINAL_KEY_TAB,
    TERMINAL_KEY_DELETE,
    TERMINAL_KEY_HOME,
    TERMINAL_KEY_END,
    TERMINAL_KEY_CTRL_C,
    TERMINAL_KEY_RESIZE,
    TERMINAL_KEY_TERMINATE
} TerminalKey;

int terminal_init(void);
void terminal_shutdown(void);

int terminal_get_size(TerminalSize *size);
int terminal_was_resized(void);

void terminal_clear(void);
void terminal_move_cursor(int row, int col);
void terminal_hide_cursor(void);
void terminal_show_cursor(void);
void terminal_write(const char *text);

int terminal_read_key(void);

#endif
