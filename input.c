#include "input.h"
#include "buffer.h"
#include "render.h"
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>  // for exit()

int visual_mode = 0;
int insert_mode = 0;
int pending_command = 0;

static char command[64];

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

void handle_input(int ch) {
    if (visual_mode) { // Visual mode handling, no Yank or Paste yet
        // Visual mode: only movement and ESC
        if (ch == 27) { // ESC
            visual_mode = 0;
            visual_start_cx = visual_start_cy = -1;
            move(cy, cx);
            draw();
            return;
        } else if ((ch == 'h' || ch == KEY_LEFT) && cx > 0) {
            cx--;
        } else if ((ch == 'l' || ch == KEY_RIGHT) && cx < strlen(buffer[cy])) {
            cx++;
        } else if ((ch == 'k' || ch == KEY_UP) && cy > 0) {
            cy--;
        } else if ((ch == 'j' || ch == KEY_DOWN) && cy < LINES - 2) {
            cy++;
        } else if (ch == 'd') { // Delete selection (characters or lines)
            if (visual_start_cx == -1 || visual_start_cy == -1) {
            visual_start_cx = cx;
            visual_start_cy = cy;
            }
            int start_y = visual_start_cy, end_y = cy;
            int start_x = visual_start_cx, end_x = cx;
            // Normalize selection
            if (start_y > end_y || (start_y == end_y && start_x > end_x)) {
            int tmp_y = start_y, tmp_x = start_x;
            start_y = end_y; start_x = end_x;
            end_y = tmp_y; end_x = tmp_x;
            }
            if (start_y == end_y) {
            // Single line: delete selected characters
            int len = strlen(buffer[start_y]);
            int del_len = end_x - start_x + 1;
            if (start_x < len && end_x < len) {
                memmove(&buffer[start_y][start_x], &buffer[start_y][end_x + 1], len - end_x);
            }
            } else {
            // Multi-line: delete from start_x to end of first line,
            // delete full lines in between, and from 0 to end_x in last line
            // 1. Remove selected part from first line
            buffer[start_y][start_x] = '\0';
            // 2. Remove selected part from last line
            int last_len = strlen(buffer[end_y]);
            if (end_x < last_len - 1) {
                memmove(buffer[end_y], &buffer[end_y][end_x + 1], last_len - end_x);
            } else {
                buffer[end_y][0] = '\0';
            }
            // 3. Remove lines in between
            for (int i = start_y + 1; i < end_y; i++) {
                buffer[i][0] = '\0';
            }
            // 4. Merge first and last line
            strcat(buffer[start_y], buffer[end_y]);
            // 5. Shift lines up to remove empty lines
            for (int i = end_y + 1; i < LINES - 1; i++) {
                strcpy(buffer[start_y + 1 + i - (end_y + 1)], buffer[i]);
            }
            // Clear the now-unused lines at the end
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
        if (ch == 27) { // ESC
            insert_mode = 0;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (cx > 0) {
                // Remove character before cursor
                memmove(&buffer[cy][cx - 1], &buffer[cy][cx], strlen(&buffer[cy][cx]) + 1);
                cx--;
            }
        } else if (ch == '\n') {
            if (cy < LINES - 2) {
            // Shift lines below down by one
            for (int i = LINES - 2; i > cy + 1; i--) {
                strcpy(buffer[i], buffer[i - 1]);
            }
            // Split the current line at cx
            int len = strlen(buffer[cy]);
            if (len > cx) {
                // Move the text after cursor to the new line
                memmove(buffer[cy + 1], &buffer[cy][cx], len - cx + 1);
                // Do not clear buffer[cy][cx], just terminate at cx
                buffer[cy][cx] = '\0';
            } else {
                buffer[cy + 1][0] = '\0';
            }
            cy++;
            cx = 0;
            }
        } else {
            if (cx < MAX_COLS - 1) {
                // Insert character at cursor position
                int len = strlen(buffer[cy]);
                if (len < MAX_COLS - 2) {
                    memmove(&buffer[cy][cx + 1], &buffer[cy][cx], len - cx + 1);
                    buffer[cy][cx++] = ch;
                }
            }
        }
    } else {
        if (pending_command) {
            if (pending_command == 'd') { // Delete command(s)
                if (ch == 'd') { // Delete current line
                    if (cy < LINES - 2) {
                        memmove(&buffer[cy], &buffer[cy + 1], (LINES - cy - 2) * MAX_COLS);
                        buffer[LINES - 2][0] = '\0';
                        if (cy > 0) cy--;
                        cx = 0;
                    }
                } else if (ch == 'w') { // Delete word under cursor
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
                cx++;
            } else if (ch == 'v') {
                visual_mode = 1;
                visual_start_cx = cx;
                visual_start_cy = cy;
                move(cy, cx);
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
            } else if (ch == ':') {  // Command mode
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