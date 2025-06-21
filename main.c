#include <ncurses.h>
#include <stdlib.h>
#include <string.h> 
#include "buffer.h"
#include "render.h"
#include "input.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    filename = argv[1];
    memset(buffer, 0, sizeof(buffer));
    load_file(filename);

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    draw();

    int ch;
    while (1) {
        ch = getch();
        handle_input(ch);
        draw();
    }

    endwin();
    return 0;
}
