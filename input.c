#include "input.h"
#include "buffer.h"
#include "render.h"
#include "config.h"
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>  // for exit()

int visual_mode = 0;
int insert_mode = 0;
int pending_command = 0;

static char command[64];

void init_input(void) {
    set_escdelay(30); // 30 ms delay (default is 1000 ms)
}

// Visual selection start position
int visual_start_cx = -1, visual_start_cy = -1;

// Helper to check if a cell is in the visual selection
int is_in_visual_selection(int y, int x) {
    if (!visual_mode || visual_start_cx == -1 || visual_start_cy == -1)
        return 0;
    int start_y = visual_start_cy, end_y = cy;
    int start_x = visual_start_cx, end_x = cx;
    if (start_y > end_y || (start_y == end_y && start_x > end_x)) {
        int tmp_y = start_y, tmp_x = start_x;
        start_y = end_y; start_x = end_x;
        end_y = tmp_y; end_x = tmp_x;
    }
    if (y < start_y || y > end_y) return 0;
    if (y == start_y && y == end_y)
        return x >= start_x && x <= end_x;
    if (y == start_y)
        return x >= start_x;
    if (y == end_y)
        return x <= end_x;
    return 1;
}

// Cursor globals (from render or main)
extern int cx, cy;

// Static vars for keys
static char key_insert = 'i';
static char key_append = 'a';
static char key_visual = 'v';
static char key_delete = 'd';
static char key_left = 'h';
static char key_right = 'l';
static char key_up = 'k';
static char key_down = 'j';
static char key_command = ':';
static char key_escape = 27; // ESC

// Declare here so main.c can call it:
void load_keys_from_config(void);

// Helper: get keybinding or fallback default (single char)
static char get_key_or_default(const char *action, char def) {
    char key = get_keybinding(action);  // now returns a char
    if (key == '\0') return def;
    return key;
}

// Call this once after load_config()
void load_keys_from_config(void) {
    key_insert  = get_key_or_default("insert_mode",  'i');
    key_append  = get_key_or_default("append_mode",  'a');
    key_visual  = get_key_or_default("visual_mode",  'v');
    key_delete  = get_key_or_default("delete",       'd');
    key_left    = get_key_or_default("move_left",    'h');
    key_right   = get_key_or_default("move_right",   'l');
    key_up      = get_key_or_default("move_up",      'k');
    key_down    = get_key_or_default("move_down",    'j');
    key_command = get_key_or_default("command_mode", ':');
}

void handle_input(int ch) {
    if (visual_mode) { // Visual mode handling
        if (ch == key_escape) {
            visual_mode = 0;
            visual_start_cx = visual_start_cy = -1;
            move(cy, cx);
            draw();
            return;
        } else if ((ch == key_left || ch == KEY_LEFT) && cx > 0) {
            cx--;
        } else if ((ch == key_right || ch == KEY_RIGHT) && cx < strlen(buffer[cy])) {
            cx++;
        } else if ((ch == key_up || ch == KEY_UP) && cy > 0) {
            cy--;
        } else if ((ch == key_down || ch == KEY_DOWN) && cy < LINES - 2) {
            cy++;
        } else if (ch == key_delete) { // Delete selection
            if (visual_start_cx == -1 || visual_start_cy == -1) {
                visual_start_cx = cx;
                visual_start_cy = cy;
            }
            int start_y = visual_start_cy, end_y = cy;
            int start_x = visual_start_cx, end_x = cx;
            if (start_y > end_y || (start_y == end_y && start_x > end_x)) {
                int tmp_y = start_y, tmp_x = start_x;
                start_y = end_y; start_x = end_x;
                end_y = tmp_y; end_x = tmp_x;
            }
            if (start_y == end_y) {
                int len = strlen(buffer[start_y]);
                if (start_x < len && end_x < len) {
                    memmove(&buffer[start_y][start_x], &buffer[start_y][end_x + 1], len - end_x);
                }
            } else {
                buffer[start_y][start_x] = '\0';
                int last_len = strlen(buffer[end_y]);
                if (end_x < last_len - 1) {
                    memmove(buffer[end_y], &buffer[end_y][end_x + 1], last_len - end_x);
                } else {
                    buffer[end_y][0] = '\0';
                }
                for (int i = start_y + 1; i < end_y; i++) {
                    buffer[i][0] = '\0';
                }
                strcat(buffer[start_y], buffer[end_y]);
                for (int i = end_y + 1; i < LINES - 1; i++) {
                    strcpy(buffer[start_y + 1 + i - (end_y + 1)], buffer[i]);
                }
                for (int i = start_y + 1 + (LINES - 1 - (end_y + 1)); i < LINES - 1; i++) {
                    buffer[i][0] = '\0';
                }
            }
            cy = start_y;
            cx = start_x;
            visual_start_cx = visual_start_cy = -1;
            visual_mode = 0;
            draw();
            return;
        }
        draw();
        return;
    }

    if (insert_mode) {
        if (ch == key_escape) {
            insert_mode = 0;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (cx > 0) {
                memmove(&buffer[cy][cx - 1], &buffer[cy][cx], strlen(&buffer[cy][cx]) + 1);
                cx--;
            }
        } else if (ch == '\n') {
            if (cy < LINES - 2) {
                for (int i = LINES - 2; i > cy + 1; i--) {
                    strcpy(buffer[i], buffer[i - 1]);
                }
                int len = strlen(buffer[cy]);
                if (len > cx) {
                    memmove(buffer[cy + 1], &buffer[cy][cx], len - cx + 1);
                    buffer[cy][cx] = '\0';
                } else {
                    buffer[cy + 1][0] = '\0';
                }
                cy++;
                cx = 0;
            }
        } else if ((ch == KEY_LEFT) && cx > 0) {
            cx--;
        } else if ((ch == KEY_RIGHT) && cx < strlen(buffer[cy])) {
            cx++;
        } else if ((ch == KEY_UP) && cy > 0) {
            cy--;
        } else if ((ch == KEY_DOWN) && cy < LINES - 2) {
            cy++;
        } else {
            if (cx < MAX_COLS - 1) {
                int len = strlen(buffer[cy]);
                if (len < MAX_COLS - 2) {
                    memmove(&buffer[cy][cx + 1], &buffer[cy][cx], len - cx + 1);
                    buffer[cy][cx++] = ch;
                }
            }
        }
    } else {
        if (pending_command) {
            if (pending_command == key_delete) {
                if (ch == key_delete) {
                    if (cy < LINES - 2) {
                        memmove(&buffer[cy], &buffer[cy + 1], (LINES - cy - 2) * MAX_COLS);
                        buffer[LINES - 2][0] = '\0';
                        if (cy > 0) cy--;
                        cx = 0;
                    }
                } else if (ch == 'w') {
                    int len = strlen(buffer[cy]);
                    while (cx < len && buffer[cy][cx] != ' ' && buffer[cy][cx] != '\0') {
                        memmove(&buffer[cy][cx], &buffer[cy][cx + 1], strlen(&buffer[cy][cx + 1]) + 1);
                        len--;
                    }
                }
            }
            pending_command = 0;
        } else {
            if (ch == key_insert) {
                insert_mode = 1;
            } else if (ch == key_append) {
                insert_mode = 1;
                cx++;
            } else if (ch == key_visual) {
                visual_mode = 1;
                visual_start_cx = cx;
                visual_start_cy = cy;
                move(cy, cx);
            } else if (ch == key_delete) {
                pending_command = key_delete;
            } else if ((ch == key_left || ch == KEY_LEFT) && cx > 0) {
                cx--;
            } else if ((ch == key_right || ch == KEY_RIGHT) && cx < strlen(buffer[cy])) {
                cx++;
            } else if ((ch == key_up || ch == KEY_UP) && cy > 0) {
                cy--;
            } else if ((ch == key_down || ch == KEY_DOWN) && cy < LINES - 2) {
                cy++;
            } else if (ch == key_command) {
                int old_cx = cx, old_cy = cy;

                move(LINES - 1, 0);
                clrtoeol();
                echo();
                printw(":");
                refresh();

                int cmd_len = 0;
                int cmd_ch;
                memset(command, 0, sizeof(command));
                while (cmd_len < (int)sizeof(command) - 1) {
                    cmd_ch = getch();
                    if (cmd_ch == key_escape) {
                        // ...
                        return;
                    } else if (cmd_ch == '\n') {
                        break;
                    } else if (cmd_ch == KEY_BACKSPACE || cmd_ch == 127) {
                        if (cmd_len > 0) {
                            cmd_len--;
                            command[cmd_len] = '\0';
                            move(LINES - 1, 1 + cmd_len);
                            delch();
                            refresh();
                        }
                    } else if (cmd_ch >= 32 && cmd_ch <= 126) { // printable
                        command[cmd_len++] = cmd_ch;
                        // No addch(cmd_ch) here!
                        refresh();
                    }
                }
                noecho();
                if (strcmp(command, "wq") == 0) {
                    save_file();
                    endwin();
                    exit(0);
                } else if (strcmp(command, "w") == 0) {
                    save_file();
                } else if (strcmp(command, "q") == 0) {
                    endwin();
                    exit(0);
                } else if (cmd_len > 0) {
                    move(LINES - 1, 0);
                    clrtoeol();
                    printw("Unknown command: %s", command);
                    refresh();
                    getch();
                }

                memset(command, 0, sizeof(command));
                cx = old_cx;
                cy = old_cy;
            }
        }
    }

    if (cx > strlen(buffer[cy])) cx = strlen(buffer[cy]);
}