#include "syntax.h"
#include <ctype.h>
#include <string.h>

// C++-specific lightweight syntax builder
void syntax_cpp_build_styles(const char *line, syntax_style_t *styles, int len) {
    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;

    int first_ns = 0;
    while (first_ns < len && isspace((unsigned char)line[first_ns])) first_ns++;
    if (first_ns < len && line[first_ns] == '#') {
        for (int j = first_ns; j < len; j++) styles[j] = STYLE_PREPROCESSOR;
        return;
    }

    int i = 0;
    while (i < len) {
        char c = line[i];
        if (c == '/' && i + 1 < len && line[i + 1] == '/') {
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }
        if (c == '/' && i + 1 < len && line[i + 1] == '*') {
            styles[i++] = STYLE_COMMENT;
            styles[i++] = STYLE_COMMENT;
            while (i < len - 1 && !(line[i] == '*' && line[i + 1] == '/')) styles[i++] = STYLE_COMMENT;
            if (i < len) { styles[i++] = STYLE_COMMENT; if (i < len) styles[i++] = STYLE_COMMENT; }
            continue;
        }
        if (c == '"') {
            styles[i++] = STYLE_STRING;
            while (i < len) {
                styles[i] = STYLE_STRING;
                if (line[i] == '"' && line[i - 1] != '\\') { i++; break; }
                i++;
            }
            continue;
        }
        if (c == '\'') {
            styles[i++] = STYLE_STRING;
            while (i < len) {
                styles[i] = STYLE_STRING;
                if (line[i] == '\'' && line[i - 1] != '\\') { i++; break; }
                i++;
            }
            continue;
        }
        if (isdigit((unsigned char)c) || (c == '.' && i + 1 < len && isdigit((unsigned char)line[i + 1]))) {
            int start = i;
            if (c == '.') i++;
            if (i + 1 < len && line[i] == '0' && (line[i + 1] == 'x' || line[i + 1] == 'X' || line[i + 1] == 'b' || line[i + 1] == 'B' || line[i + 1] == 'o' || line[i + 1] == 'O')) {
                i += 2;
                while (i < len && (isxdigit((unsigned char)line[i]) || line[i] == '_')) i++;
            } else {
                while (i < len && (isdigit((unsigned char)line[i]) || line[i] == '.' || line[i] == 'e' || line[i] == 'E' || line[i] == 'f' || line[i] == 'F' || line[i] == 'l' || line[i] == 'L' || line[i] == 'u' || line[i] == 'U' || line[i] == '_')) i++;
            }
            for (int j = start; j < i; j++) styles[j] = STYLE_NUMBER;
            continue;
        }
        if (isalpha((unsigned char)c) || c == '_') {
            int start = i; i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
            int ident_len = i - start;
            char ident[128];
            if (ident_len >= (int)sizeof(ident)) ident_len = (int)sizeof(ident) - 1;
            memcpy(ident, &line[start], ident_len);
            ident[ident_len] = '\0';
            static const char *keywords[] = {
                "alignas","alignof","and","and_eq","asm","auto","bool","break","case","catch",
                "char","class","compl","const","const_cast","continue","decltype","default","delete",
                "do","double","dynamic_cast","else","enum","explicit","export","extern","false","float",
                "for","friend","goto","if","inline","int","long","mutable","namespace","new","noexcept",
                "not","not_eq","nullptr","operator","or","or_eq","private","protected","public","register",
                "reinterpret_cast","return","short","signed","sizeof","static","static_assert","static_cast",
                "struct","switch","template","this","thread_local","throw","true","try","typedef","typeid",
                "typename","union","unsigned","using","virtual","void","volatile","wchar_t","while","xor","xor_eq"
            };
            int is_kw = 0;
            for (size_t k = 0; k < sizeof(keywords) / sizeof(keywords[0]); k++) {
                if (strcmp(ident, keywords[k]) == 0) { is_kw = 1; break; }
            }
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
