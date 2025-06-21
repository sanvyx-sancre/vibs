#include "render.h"
#include "buffer.h"
#include <ncurses.h>
#include <string.h>

int insert_mode = 0;
int pending_command = 0;

void draw(void) {
    clear();
    for (int i = 0; i < LINES - 1; i++) {
        if (strlen(buffer[i]) > 0)
            mvprintw(i, 0, "%s", buffer[i]);
        else
            mvprintw(i, 0, "~");
    }

    mvprintw(LINES - 1, 0, "-- %s -- | Line %d Col %d | File: %s",
             insert_mode ? "INSERT" : "NORMAL",
             cy + 1, cx + 1,
             filename ? filename : "(no file)");
    move(cy, cx);
    refresh();
}
