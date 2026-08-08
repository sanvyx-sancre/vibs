#include "syntax.h"
#include <ctype.h>
#include <string.h>

// Python-specific lightweight syntax builder
void syntax_python_build_styles(const char *line, syntax_style_t *styles, int len) {
    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;

    int i = 0;
    while (i < len) {
        char c = line[i];
        // comment starts with # to end of line
        if (c == '#') {
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }
        // string (single- or double-quoted) - simple
        if (c == '"' || c == '\'') {
            char q = c;
            styles[i++] = STYLE_STRING;
            while (i < len) {
                styles[i] = STYLE_STRING;
                if (line[i] == q && line[i - 1] != '\\') { i++; break; }
                i++;
            }
            continue;
        }
        // numbers
        if (isdigit((unsigned char)c) || (c == '.' && i + 1 < len && isdigit((unsigned char)line[i + 1]))) {
            int start = i; if (c == '.') i++;
            while (i < len && (isdigit((unsigned char)line[i]) || line[i] == '.')) i++;
            for (int j = start; j < i; j++) styles[j] = STYLE_NUMBER;
            continue;
        }
        // identifiers and keywords
        if (isalpha((unsigned char)c) || c == '_') {
            int start = i; i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
            // function call heuristic
            if (i < len && line[i] == '(') {
                for (int j = start; j < i; j++) styles[j] = STYLE_KEYWORD;
            }
            continue;
        }
        i++;
    }
}
