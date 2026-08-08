#include "render.h"
#include "buffer.h"
#include "input.h"
#include "config.h"
#include "syntax.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

extern int status_line_visible;

// Forward declaration for selection check
int is_in_visual_selection(int y, int x);

static void draw_text_row(int row, int buf_line) {
    syntax_init_colors();

    int gutter_width = show_line_numbers() ? 5 : 0;
    int content_col = show_line_numbers() ? gutter_width + 1 : 0;
    int visible_cols = COLS > content_col ? COLS - content_col : 1;
    int len = strlen(buffer[buf_line]);
    int draw_len = len < visible_cols ? len : visible_cols;
    int is_cursor_row = (buf_line == cy && row == cy - screen_top + 1);

    if (show_line_numbers()) {
        char line_num[16];
        snprintf(line_num, sizeof(line_num), "%*d ", gutter_width, buf_line + 1);
        attron(A_DIM);
        mvprintw(row, 0, "%s", line_num);
        attroff(A_DIM);
    }

    syntax_style_t styles[1024];
    syntax_set_mode(syntax_detect_mode(filename));
    syntax_build_styles(buffer[buf_line], styles, len);

    if (len > 0) {
        for (int j = 0; j < draw_len; j++) {
            int is_cursor_cell = is_cursor_row && j == cx;
            int style = styles[j];

            if (is_cursor_cell) {
                attron(A_REVERSE | A_BOLD);
            } else if (visual_mode && is_in_visual_selection(buf_line, j)) {
                attron(A_REVERSE);
            } else if (style == STYLE_KEYWORD) {
                attron(COLOR_PAIR(1));
            } else if (style == STYLE_STRING) {
                attron(COLOR_PAIR(2));
            } else if (style == STYLE_COMMENT) {
                attron(COLOR_PAIR(3));
            } else if (style == STYLE_NUMBER) {
                attron(COLOR_PAIR(4));
            } else if (style == STYLE_PREPROCESSOR) {
                attron(COLOR_PAIR(5));
            }

            mvaddch(row, content_col + j, buffer[buf_line][j]);

            if (is_cursor_cell || (visual_mode && is_in_visual_selection(buf_line, j))) {
                attroff(A_REVERSE | A_BOLD);
            } else if (style == STYLE_KEYWORD || style == STYLE_STRING || style == STYLE_COMMENT || style == STYLE_NUMBER || style == STYLE_PREPROCESSOR) {
                attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(4) | COLOR_PAIR(5));
            }
        }
    } else {
        mvaddch(row, content_col, '~');
    }

    for (int j = draw_len; j < visible_cols; j++) {
        mvaddch(row, content_col + j, ' ');
    }
}

void draw(void) {
    clear();

    const char *file_name = filename ? filename : "(new file)";
    char header[256];
    snprintf(header, sizeof(header), " vibs • %s ", file_name);

    attron(A_BOLD);
    mvprintw(0, 0, "%s", header);
    attroff(A_BOLD);
    clrtoeol();

    for (int i = 1; i < LINES - 1; i++) {
        int buf_line = screen_top + (i - 1);
        if (buf_line >= MAX_LINES) break;
        draw_text_row(i, buf_line);
    }

    const char *mode_str = "NORMAL";
    if (insert_mode)
        mode_str = "INSERT";
    else if (visual_mode)
        mode_str = "VISUAL";

    if (status_line_visible) {
        attron(A_BOLD);
        mvprintw(LINES - 1, 0, " %s | %s | line %d:%d ", mode_str, file_name, cy + 1, cx + 1);
        attroff(A_BOLD);
        clrtoeol();
    } else {
        mvprintw(LINES - 1, 0, "  [status hidden] ");
        clrtoeol();
    }

    int gutter_width = show_line_numbers() ? 5 : 0;
    int content_col = show_line_numbers() ? gutter_width + 1 : 0;
    int visible_cols = COLS > content_col ? COLS - content_col : 1;
    int cursor_row = cy - screen_top + 1;
    int cursor_col = cx < visible_cols ? (show_line_numbers() ? content_col + cx : cx) : (show_line_numbers() ? content_col + visible_cols - 1 : visible_cols - 1);
    if (cursor_row < 1) cursor_row = 1;
    if (cursor_row >= LINES - 1) cursor_row = LINES - 2;

    move(cursor_row, cursor_col);
    refresh();
}