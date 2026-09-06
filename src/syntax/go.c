#include "syntax.h"
#include <ctype.h>
#include <string.h>

// Go-specific lightweight syntax builder
void syntax_go_build_styles(const char *line, syntax_style_t *styles, int len) {
    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;

    static const char *keywords[] = {
        "break", "default", "func", "interface", "select", "case", "defer", "go",
        "map", "struct", "chan", "else", "goto", "package", "switch", "const",
        "fallthrough", "if", "range", "type", "continue", "for", "import", "return",
        "var"
    };
    static const char *types[] = {
        "bool", "byte", "complex64", "complex128", "error", "float32", "float64",
        "int", "int8", "int16", "int32", "int64", "rune", "string", "uint", "uint8",
        "uint16", "uint32", "uint64", "uintptr"
    };

    int i = 0;
    while (i < len) {
        char c = line[i];

        // Go line comments and per-line block comments.
        if (c == '/' && i + 1 < len && line[i + 1] == '/') {
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }
        if (c == '/' && i + 1 < len && line[i + 1] == '*') {
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }

        // Interpreted strings, rune literals, and raw backtick strings.
        if (c == '"' || c == '\'' || c == '`') {
            char quote = c;
            int start = i++;
            while (i < len) {
                if (line[i] == quote && (i == start + 1 || line[i - 1] != '\\' || quote == '`')) { i++; break; }
                i++;
            }
            for (int j = start; j < i; j++) styles[j] = STYLE_STRING;
            continue;
        }

        // Decimal, hexadecimal, binary, octal, and floating-point literals.
        if (isdigit((unsigned char)c) || (c == '.' && i + 1 < len && isdigit((unsigned char)line[i + 1]))) {
            int start = i++;
            if (c == '0' && i < len && (line[i] == 'x' || line[i] == 'X' || line[i] == 'b' || line[i] == 'B' || line[i] == 'o' || line[i] == 'O')) i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_' || line[i] == '.')) i++;
            for (int j = start; j < i; j++) styles[j] = STYLE_NUMBER;
            continue;
        }

        // Keywords, built-in types, and function-call names.
        if (isalpha((unsigned char)c) || c == '_') {
            int start = i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
            int word_len = i - start;
            char word[64];
            if (word_len >= (int)sizeof(word)) word_len = (int)sizeof(word) - 1;
            memcpy(word, &line[start], word_len);
            word[word_len] = '\0';

            int styled = 0;
            for (size_t k = 0; k < sizeof(keywords) / sizeof(keywords[0]); k++) {
                if (strcmp(word, keywords[k]) == 0) { styled = 1; break; }
            }
            for (size_t k = 0; !styled && k < sizeof(types) / sizeof(types[0]); k++) {
                if (strcmp(word, types[k]) == 0) { styled = 1; break; }
            }
            if (styled || (i < len && line[i] == '(')) {
                for (int j = start; j < i; j++) styles[j] = STYLE_KEYWORD;
            }
            continue;
        }

        i++;
    }
}