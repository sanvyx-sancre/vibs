#include "syntax.h"
#include <ctype.h>
#include <string.h>

// Python-specific lightweight syntax builder
void syntax_python_build_styles(const char *line, syntax_style_t *styles, int len) {
    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;
    // first non-space check for comments or decorators
    int first_ns = 0;
    while (first_ns < len && isspace((unsigned char)line[first_ns])) first_ns++;
    if (first_ns < len) {
        if (line[first_ns] == '#') {
            for (int j = first_ns; j < len; j++) styles[j] = STYLE_COMMENT;
            return;
        }
        if (line[first_ns] == '@') {
            // decorator style
            int j = first_ns;
            while (j < len && !isspace((unsigned char)line[j])) styles[j++] = STYLE_PREPROCESSOR;
        }
    }

    int i = 0;
    while (i < len) {
        char c = line[i];
        // string prefixes like r, u, f, b possibly before quote
        if ((c == 'r' || c == 'R' || c == 'u' || c == 'U' || c == 'f' || c == 'F' || c == 'b' || c == 'B') && i + 1 < len && (line[i+1] == '"' || line[i+1] == '\'')) {
            // mark prefix + string
            int p = i;
            styles[p++] = STYLE_STRING; // prefix
            i = p;
            // fall through to string handler
        }
        if (line[i] == '"' || line[i] == '\'') {
            char q = line[i];
            // triple-quote?
            if (i + 2 < len && line[i+1] == q && line[i+2] == q) {
                // triple-quoted string
                styles[i++] = STYLE_STRING; styles[i++] = STYLE_STRING; styles[i++] = STYLE_STRING;
                while (i + 2 < len) {
                    styles[i] = STYLE_STRING;
                    if (line[i] == q && line[i+1] == q && line[i+2] == q) {
                        styles[i++] = STYLE_STRING;
                        styles[i++] = STYLE_STRING;
                        styles[i++] = STYLE_STRING;
                        break;
                    }
                    i++;
                }
                continue;
            }
            // normal single-line string
            styles[i++] = STYLE_STRING;
            while (i < len) {
                styles[i] = STYLE_STRING;
                if (line[i] == q && i > 0 && line[i-1] != '\\') { i++; break; }
                i++;
            }
            continue;
        }
        // numbers (dec, hex, bin, oct, floats)
        if (isdigit((unsigned char)c) || (c == '.' && i + 1 < len && isdigit((unsigned char)line[i+1]))) {
            int start = i;
            if (c == '.') i++;
            if (i + 1 < len && line[i] == '0' && (line[i+1]=='x' || line[i+1]=='X' || line[i+1]=='b' || line[i+1]=='B' || line[i+1]=='o' || line[i+1]=='O')) {
                i += 2; while (i < len && (isxdigit((unsigned char)line[i]) || line[i]=='_')) i++;
            } else {
                while (i < len && (isdigit((unsigned char)line[i]) || line[i]=='.' || line[i]=='e' || line[i]=='E' || line[i]=='_' || line[i]=='j' || line[i]=='J')) i++;
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
            static const char *keywords[] = {
                "and","as","assert","break","class","continue","def","del",
                "elif","else","except","False","finally","for","from","global",
                "if","import","in","is","lambda","None","nonlocal","not","or",
                "pass","raise","return","True","try","while","with","yield","async","await"
            };
            int is_kw = 0;
            for (size_t k = 0; k < sizeof(keywords)/sizeof(keywords[0]); k++) if (strcmp(ident, keywords[k])==0) { is_kw = 1; break; }
            if (is_kw) {
                for (int j = start; j < i; j++) styles[j] = STYLE_KEYWORD;
            } else if (i < len && line[i] == '(') {
                for (int j = start; j < i; j++) styles[j] = STYLE_KEYWORD;
            }
            continue;
        }
        i++;
    }
}
