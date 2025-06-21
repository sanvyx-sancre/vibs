#include "input.h"
#include "buffer.h"
#include "render.h"
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>  // for exit()


extern int insert_mode;
extern int pending_command;

static char command[64];

void handle_input(int ch) {
    if (insert_mode) {
        if (ch == 27) { // ESC
            insert_mode = 0;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (cx > 0) {
                buffer[cy][--cx] = '\0';
            }
        } else if (ch == '\n') {
            if (cy < LINES - 2) {
                cy++;
                cx = 0;
                buffer[cy][0] = '\0';
            }
        } else {
            if (cx < MAX_COLS - 1) {
                buffer[cy][cx++] = ch;
                buffer[cy][cx] = '\0';
            }
        }
    } else {
        if (pending_command) {
            if (pending_command == 'd') {
                if (ch == 'd') {
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
            if (ch == 'i') {
                insert_mode = 1;
            } else if (ch == 'a') {
                insert_mode = 1;
                cx = strlen(buffer[cy]);
            } else if (ch == 'd') {
                pending_command = 'd';
            } else if ((ch == 'h' || ch == KEY_LEFT) && cx > 0) {
                cx--;
            } else if ((ch == 'l' || ch == KEY_RIGHT) && cx < strlen(buffer[cy])) {
                cx++;
            } else if ((ch == 'k' || ch == KEY_UP) && cy > 0) {
                cy--;
            } else if ((ch == 'j' || ch == KEY_DOWN) && cy < LINES - 2) {
                cy++;
            } else if (ch == ':') {
                int old_cx = cx, old_cy = cy;

                move(LINES - 1, 0);
                clrtoeol();
                echo();
                printw(":");
                refresh();
                getnstr(command, sizeof(command) - 1);
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
                } else {
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
