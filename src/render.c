#include "render.h"
#include "buffer.h"
#include "input.h"
#include "config.h"
#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

extern int status_line_visible;

// Forward declaration for selection check
int is_in_visual_selection(int y, int x);

typedef enum {
    STYLE_NORMAL,
    STYLE_KEYWORD,
    STYLE_STRING,
    STYLE_COMMENT,
    STYLE_NUMBER,
    STYLE_PREPROCESSOR
} syntax_style_t;

static int syntax_colors_ready = 0;

static void init_syntax_colors(void) {
    if (syntax_colors_ready) return;
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_CYAN, -1);
        init_pair(2, COLOR_GREEN, -1);
        init_pair(3, COLOR_MAGENTA, -1);
        init_pair(4, COLOR_YELLOW, -1);
        init_pair(5, COLOR_BLUE, -1);
    }
    syntax_colors_ready = 1;
}

static int is_c_keyword(const char *word) {
    static const char *keywords[] = {
        "auto", "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if",
        "int", "long", "register", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union", "unsigned", "void", "volatile",
        "while", "bool", "true", "false"
    };
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void build_syntax_styles(const char *line, syntax_style_t *styles, int len) {
    for (int i = 0; i < len; i++) {
        styles[i] = STYLE_NORMAL;
    }

    int i = 0;
    while (i < len) {
        char c = line[i];
        if (c == '/' && i + 1 < len && line[i + 1] == '/') {
            while (i < len) {
                styles[i++] = STYLE_COMMENT;
            }
            break;
        }

        if (c == '/' && i + 1 < len && line[i + 1] == '*') {
            styles[i++] = STYLE_COMMENT;
            styles[i++] = STYLE_COMMENT;
            while (i < len - 1 && !(line[i] == '*' && line[i + 1] == '/')) {
                styles[i++] = STYLE_COMMENT;
            }
            if (i < len) {
                styles[i++] = STYLE_COMMENT;
                styles[i++] = STYLE_COMMENT;
            }
            continue;
        }

        if (c == '"') {
            styles[i++] = STYLE_STRING;
            while (i < len) {
                styles[i] = STYLE_STRING;
                if (line[i] == '"' && line[i - 1] != '\\') {
                    i++;
                    break;
                }
                i++;
            }
            continue;
        }

        if (c == '\'') {
            styles[i++] = STYLE_STRING;
            while (i < len) {
                styles[i] = STYLE_STRING;
                if (line[i] == '\'' && line[i - 1] != '\\') {
                    i++;
                    break;
                }
                i++;
            }
            continue;
        }

        if (isdigit((unsigned char)c) || (c == '.' && i + 1 < len && isdigit((unsigned char)line[i + 1]))) {
            int start = i;
            if (c == '.') {
                i++;
            }
            while (i < len && (isdigit((unsigned char)line[i]) || line[i] == '.' || line[i] == 'e' || line[i] == 'E' || line[i] == 'x' || line[i] == 'X' || line[i] == 'f' || line[i] == 'F' || line[i] == 'l' || line[i] == 'L' || line[i] == 'u' || line[i] == 'U')) {
                i++;
            }
            for (int j = start; j < i; j++) {
                styles[j] = STYLE_NUMBER;
            }
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_') {
            int start = i;
            i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) {
                i++;
            }
            char ident[64];
            int ident_len = i - start;
            if (ident_len >= (int)sizeof(ident)) {
                ident_len = (int)sizeof(ident) - 1;
            }
            memcpy(ident, &line[start], ident_len);
            ident[ident_len] = '\0';
            if (ident_len > 0 && is_c_keyword(ident)) {
                for (int j = start; j < i; j++) {
                    styles[j] = STYLE_KEYWORD;
                }
            } else if (start == 0 && ident_len > 0 && line[start] == '#') {
                for (int j = start; j < i; j++) {
                    styles[j] = STYLE_PREPROCESSOR;
                }
            }
            continue;
        }

        i++;
    }
}

static void draw_text_row(int row, int buf_line) {
    init_syntax_colors();

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
    build_syntax_styles(buffer[buf_line], styles, len);

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