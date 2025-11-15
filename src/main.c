#include <ncurses.h>
#include <stdlib.h>
#include <string.h> 
#include "buffer.h"
#include "render.h"
#include "input.h"
#include "config.h"

int main(int argc, char *argv[]) {
    const char* home = getenv("HOME");
    if (!home) {
        printf("Error: HOME environment variable not set.\n");
        return 1;
    }

    char config_path[512];
    snprintf(config_path, sizeof(config_path), "%s/.config/vibs/config.toml", home); // fixed finally

    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    memset(buffer, 0, sizeof(buffer));

    // use the dynamic path here
    load_config(config_path);
    load_keys_from_config();

    if (strcmp(argv[1], "-config") == 0) {
        load_config(config_path);
        load_keys_from_config();
        return 0;
    }

    load_file(argv[1]);

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
