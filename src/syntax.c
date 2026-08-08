#include "syntax.h"
#include <ctype.h>
#include <ncurses.h>
#include <string.h>

static int syntax_colors_ready = 0;

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

void syntax_init_colors(void) {
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

void syntax_build_styles(const char *line, syntax_style_t *styles, int len) {
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
