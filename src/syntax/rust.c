#include "syntax.h"
#include <ctype.h>
#include <string.h>

// Rust-specific lightweight syntax builder
void syntax_rust_build_styles(const char *line, syntax_style_t *styles, int len) {
    static const char *keywords[] = {
        "as", "async", "await", "break", "const", "continue", "crate", "dyn",
        "else", "enum", "extern", "false", "fn", "for", "if", "impl", "in",
        "let", "loop", "match", "mod", "move", "mut", "pub", "ref", "return",
        "self", "Self", "static", "struct", "super", "trait", "true", "type",
        "unsafe", "use", "where", "while", "yield", "try"
    };
    static const char *types[] = {
        "bool", "char", "str", "u8", "u16", "u32", "u64", "u128", "usize",
        "i8", "i16", "i32", "i64", "i128", "isize", "f32", "f64"
    };

    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;

    int i = 0;
    while (i < len) {
        char c = line[i];

        // Attributes and inner attributes, such as #[derive(...)] and #![no_std].
        if (c == '#' && i + 1 < len && (line[i + 1] == '[' || line[i + 1] == '!')) {
            while (i < len && line[i] != ']') styles[i++] = STYLE_PREPROCESSOR;
            if (i < len) styles[i++] = STYLE_PREPROCESSOR;
            continue;
        }

        // Line and block comments. Block comments are per-line only here.
        if (c == '/' && i + 1 < len && line[i + 1] == '/') {
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }
        if (c == '/' && i + 1 < len && line[i + 1] == '*') {
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }

        // Raw strings: r"...", r#"..."#, and similar forms.
        if (c == 'r' && i + 1 < len) {
            int prefix_end = i + 1;
            while (prefix_end < len && line[prefix_end] == '#') prefix_end++;
            if (prefix_end < len && line[prefix_end] == '"') {
                int hashes = prefix_end - (i + 1);
                int start = i;
                i = prefix_end + 1;
                while (i < len) {
                    if (line[i] == '"') {
                        int p = i + 1;
                        int count = 0;
                        while (p < len && line[p] == '#' && count < hashes) { count++; p++; }
                        if (count == hashes) { i = p; break; }
                    }
                    i++;
                }
                for (int j = start; j < i && j < len; j++) styles[j] = STYLE_STRING;
                continue;
            }
        }

        // Normal strings and character literals, including escaped delimiters.
        if (c == '"' || c == '\'') {
            char quote = c;
            int start = i++;
            while (i < len) {
                if (line[i] == quote && line[i - 1] != '\\') { i++; break; }
                i++;
            }
            for (int j = start; j < i && j < len; j++) styles[j] = STYLE_STRING;
            continue;
        }

        // Decimal, hexadecimal, binary, octal, and floating-point literals.
        if (isdigit((unsigned char)c) || (c == '.' && i + 1 < len && isdigit((unsigned char)line[i + 1]))) {
            int start = i++;
            if (c == '0' && i < len && (line[i] == 'x' || line[i] == 'X' || line[i] == 'b' || line[i] == 'B' || line[i] == 'o' || line[i] == 'O')) i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_' || line[i] == '.')) i++;
            for (int j = start; j < i && j < len; j++) styles[j] = STYLE_NUMBER;
            continue;
        }

        // Keywords, primitive types, and function-call names.
        if (isalpha((unsigned char)c) || c == '_') {
            int start = i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
            int word_len = i - start;
            char word[64];
            if (word_len >= (int)sizeof(word)) word_len = (int)sizeof(word) - 1;
            memcpy(word, &line[start], word_len);
            word[word_len] = '\0';

            int is_keyword = 0;
            for (size_t k = 0; k < sizeof(keywords) / sizeof(keywords[0]); k++) {
                if (strcmp(word, keywords[k]) == 0) { is_keyword = 1; break; }
            }
            for (size_t k = 0; !is_keyword && k < sizeof(types) / sizeof(types[0]); k++) {
                if (strcmp(word, types[k]) == 0) { is_keyword = 1; break; }
            }
            if (is_keyword || (i < len && line[i] == '(')) {
                for (int j = start; j < i; j++) styles[j] = STYLE_KEYWORD;
            }
            continue;
        }

        i++;
    }
}