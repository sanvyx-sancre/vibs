#include "render.h"
#include "buffer.h"
#include "input.h"
#include <ncurses.h>
#include <string.h>

// Forward declaration for selection check
int is_in_visual_selection(int y, int x);

void draw(void) {
    clear();
    for (int i = 0; i < LINES - 1; i++) {
        int len = strlen(buffer[i]);
        if (len > 0) {
            for (int j = 0; j < len; j++) {
                if (visual_mode && is_in_visual_selection(i, j)) {
                    attron(A_REVERSE);
                    mvaddch(i, j, buffer[i][j]);
                    attroff(A_REVERSE);
                } else {
                    mvaddch(i, j, buffer[i][j]);
                }
            }
        } else {
            mvprintw(i, 0, "~");
        }
    }

    const char *mode_str = "NORMAL";
    if (insert_mode)
        mode_str = "INSERT";
    else if (visual_mode)
        mode_str = "VISUAL";

    mvprintw(LINES - 1, 0, "-- %s -- | Line %d Col %d | File: %s",
             mode_str,
             cy + 1, cx + 1,
             filename ? filename : "(no file)");
    move(cy, cx);
    refresh();
}