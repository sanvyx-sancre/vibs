#include "syntax.h"
#include <ctype.h>
#include <string.h>

// C-specific lightweight syntax builder
void syntax_c_build_styles(const char *line, syntax_style_t *styles, int len) {
    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;
    // quick check: if line is a preprocessor directive (#) mark it as PREPROCESSOR
    int first_ns = 0;
    while (first_ns < len && isspace((unsigned char)line[first_ns])) first_ns++;
    if (first_ns < len && line[first_ns] == '#') {
        for (int j = first_ns; j < len; j++) styles[j] = STYLE_PREPROCESSOR;
        return;
    }

    int i = 0;
    while (i < len) {
        char c = line[i];
        // single-line comment
        if (c == '/' && i + 1 < len && line[i + 1] == '/') {
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }
        // block comment
        if (c == '/' && i + 1 < len && line[i + 1] == '*') {
            styles[i++] = STYLE_COMMENT;
            styles[i++] = STYLE_COMMENT;
            while (i < len - 1 && !(line[i] == '*' && line[i + 1] == '/')) styles[i++] = STYLE_COMMENT;
            if (i < len) { styles[i++] = STYLE_COMMENT; if (i < len) styles[i++] = STYLE_COMMENT; }
            continue;
        }
        // string literal
        if (c == '"') {
            styles[i++] = STYLE_STRING;
            while (i < len) {
                styles[i] = STYLE_STRING;
                if (line[i] == '"' && line[i - 1] != '\\') { i++; break; }
                i++;
            }
            continue;
        }
        // char literal
        if (c == '\'') {
            styles[i++] = STYLE_STRING;
            while (i < len) {
                styles[i] = STYLE_STRING;
                if (line[i] == '\'' && line[i - 1] != '\\') { i++; break; }
                i++;
            }
            continue;
        }
        // numbers (decimal, hex, oct, bin, floats)
        if (isdigit((unsigned char)c) || (c == '.' && i + 1 < len && isdigit((unsigned char)line[i + 1]))) {
            int start = i;
            if (c == '.') i++;
            if (i + 1 < len && line[i] == '0' && (line[i+1] == 'x' || line[i+1] == 'X' || line[i+1] == 'b' || line[i+1] == 'B' || line[i+1] == 'o' || line[i+1] == 'O')) {
                // hex/bin/oct
                i += 2;
                while (i < len && (isxdigit((unsigned char)line[i]) || strchr("_", line[i]))) i++;
            } else {
                while (i < len && (isdigit((unsigned char)line[i]) || line[i] == '.' || line[i] == 'e' || line[i] == 'E' || line[i] == 'f' || line[i] == 'F' || line[i] == 'l' || line[i] == 'L' || line[i] == 'u' || line[i] == 'U' || line[i] == '_')) i++;
            }
            for (int j = start; j < i; j++) styles[j] = STYLE_NUMBER;
            continue;
        }
        // identifiers / keywords / function names
        if (isalpha((unsigned char)c) || c == '_') {
            int start = i; i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
            int ident_len = i - start;
            char ident[128];
            if (ident_len >= (int)sizeof(ident)) ident_len = (int)sizeof(ident) - 1;
            memcpy(ident, &line[start], ident_len);
            ident[ident_len] = '\0';
            // keyword check (expanded)
            static const char *keywords[] = {
                "auto","break","case","char","const","continue","default","do",
                "double","else","enum","extern","float","for","goto","if",
                "int","long","register","return","short","signed","sizeof","static",
                "struct","switch","typedef","union","unsigned","void","volatile",
                "while","_Bool","inline","restrict","const_cast","static_cast","offsetof","NULL"
            };
            int is_kw = 0;
            for (size_t k = 0; k < sizeof(keywords)/sizeof(keywords[0]); k++) {
                if (strcmp(ident, keywords[k]) == 0) { is_kw = 1; break; }
            }
            if (is_kw) {
                for (int j = start; j < i; j++) styles[j] = STYLE_KEYWORD;
            } else if (i < len && line[i] == '(') {
                // simple heuristic: treat ident followed by '(' as function (style keyword)
                for (int j = start; j < i; j++) styles[j] = STYLE_KEYWORD;
            }
            continue;
        }
        i++;
    }
}
