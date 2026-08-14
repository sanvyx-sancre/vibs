#include "syntax.h"
#include <ctype.h>
#include <string.h>

// Minimal Lua syntax highlighter (per-line)
void syntax_lua_build_styles(const char *line, syntax_style_t *styles, int len) {
    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;

    // shebang
    if (len > 1 && line[0] == '#' && line[1] == '!') {
        for (int j = 0; j < len; j++) styles[j] = STYLE_COMMENT;
        return;
    }

    int i = 0;
    while (i < len) {
        char c = line[i];

        // comments: -- or --[[
        if (c == '-' && i + 1 < len && line[i+1] == '-') {
            // if long bracket comment start '--[' mark rest of line as comment (per-line)
            if (i + 2 < len && line[i+2] == '[') {
                while (i < len) styles[i++] = STYLE_COMMENT;
                break;
            }
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }

        // long bracket string start [[ or [=[ ... ]=]
        if (c == '[' && i + 1 < len && line[i+1] == '[') {
            int start = i;
            i += 2;
            while (i < len) {
                if (line[i] == ']' && i + 1 < len && line[i+1] == ']') { i += 2; break; }
                i++;
            }
            for (int j = start; j < i && j < len; j++) styles[j] = STYLE_STRING;
            continue;
        }

        // string literals ' or "
        if (c == '\'' || c == '"') {
            char q = c;
            int start = i;
            i++;
            while (i < len) {
                if (line[i] == q && line[i-1] != '\\') { i++; break; }
                i++;
            }
            for (int j = start; j < i && j < len; j++) styles[j] = STYLE_STRING;
            continue;
        }

        // numbers
        if (isdigit((unsigned char)c) || (c == '.' && i + 1 < len && isdigit((unsigned char)line[i+1]))) {
            int start = i;
            if (c == '.') i++;
            if (i + 1 < len && line[i] == '0' && (line[i+1] == 'x' || line[i+1] == 'X')) {
                i += 2; while (i < len && (isxdigit((unsigned char)line[i]) || line[i] == '_')) i++;
            } else {
                while (i < len && (isdigit((unsigned char)line[i]) || line[i] == '.' || line[i] == 'e' || line[i] == 'E' || line[i] == '_')) i++;
            }
            for (int j = start; j < i && j < len; j++) styles[j] = STYLE_NUMBER;
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
            static const char *keywords[] = {
                "and","break","do","else","elseif","end","false","for","function",
                "goto","if","in","local","nil","not","or","repeat","return","then",
                "true","until","while"
            };
            int is_kw = 0;
            for (size_t k = 0; k < sizeof(keywords)/sizeof(keywords[0]); k++) if (strcmp(ident, keywords[k])==0) { is_kw = 1; break; }
            if (is_kw) {
                for (int j = start; j < i && j < len; j++) styles[j] = STYLE_KEYWORD;
            } else if (i < len && line[i] == '(') {
                for (int j = start; j < i && j < len; j++) styles[j] = STYLE_KEYWORD;
            }
            continue;
        }

        i++;
    }
}
