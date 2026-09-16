#include "syntax.h"
#include <ctype.h>
#include <string.h>

// Architecture-neutral assembly syntax builder.
void syntax_asm_build_styles(const char *line, syntax_style_t *styles, int len) {
    static const char *mnemonics[] = {
        "adc", "add", "and", "asr", "b", "bl", "beq", "bne", "bgt", "blt",
        "call", "cmp", "dec", "div", "idiv", "imul", "inc", "jmp", "jz", "jnz",
        "je", "jne", "jg", "jge", "jl", "jle", "lea", "ldr", "lsl", "lsr",
        "mov", "movq", "movl", "mul", "neg", "nop", "not", "or", "pop", "push",
        "ret", "ror", "rol", "sar", "sbc", "sdiv", "shl", "shr", "stp", "str",
        "sub", "svc", "syscall", "test", "udiv", "xor"
    };
    static const char *registers[] = {
        "al", "ah", "ax", "eax", "rax", "bl", "bh", "bx", "ebx", "rbx",
        "cl", "ch", "cx", "ecx", "rcx", "dl", "dh", "dx", "edx", "rdx",
        "rsi", "rdi", "rsp", "rbp", "rip", "sp", "lr", "pc",
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9",
        "r10", "r11", "r12", "x0", "x1", "x2", "x3", "x4", "x5", "x6",
        "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16",
        "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26",
        "x27", "x28", "x29", "x30", "w0", "w1", "w2", "w3", "w4", "w5",
        "w6", "w7", "w8", "w9", "w10", "w11", "w12", "w13", "w14", "w15",
        "w16", "w17", "w18", "w19", "w20", "w21", "w22", "w23", "w24", "w25",
        "w26", "w27", "w28", "w29", "w30"
    };

    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;

    int i = 0;
    while (i < len) {
        char c = line[i];

        // Common comment markers: ';', '#', and //.
        if (c == ';' || c == '#') {
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }
        if (c == '/' && i + 1 < len && line[i + 1] == '/') {
            while (i < len) styles[i++] = STYLE_COMMENT;
            break;
        }

        // Quoted strings and character literals.
        if (c == '"' || c == '\'') {
            char quote = c;
            int start = i++;
            while (i < len) {
                if (line[i] == quote && line[i - 1] != '\\') { i++; break; }
                i++;
            }
            for (int j = start; j < i; j++) styles[j] = STYLE_STRING;
            continue;
        }

        // Decimal, hexadecimal, binary, and octal constants.
        if (isdigit((unsigned char)c) || (c == '0' && i + 1 < len && (line[i + 1] == 'x' || line[i + 1] == 'X'))) {
            int start = i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_' || line[i] == '.')) i++;
            for (int j = start; j < i; j++) styles[j] = STYLE_NUMBER;
            continue;
        }

        // Directives begin with a dot; labels end with a colon.
        if (c == '.') {
            int start = i++;
            while (i < len && (isalpha((unsigned char)line[i]) || line[i] == '_')) i++;
            if (i > start + 1) {
                for (int j = start; j < i; j++) styles[j] = STYLE_PREPROCESSOR;
                continue;
            }
            i = start;
        }

        if (isalpha((unsigned char)c) || c == '_') {
            int start = i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
            int word_len = i - start;
            char word[64];
            if (word_len >= (int)sizeof(word)) word_len = (int)sizeof(word) - 1;
            memcpy(word, &line[start], word_len);
            word[word_len] = '\0';

            int styled = 0;
            for (size_t k = 0; k < sizeof(mnemonics) / sizeof(mnemonics[0]); k++) {
                if (strcmp(word, mnemonics[k]) == 0) { styled = 1; break; }
            }
            for (size_t k = 0; !styled && k < sizeof(registers) / sizeof(registers[0]); k++) {
                if (strcmp(word, registers[k]) == 0) { styled = 1; break; }
            }
            if (styled) {
                for (int j = start; j < i; j++) styles[j] = STYLE_KEYWORD;
            } else if (i < len && line[i] == ':') {
                for (int j = start; j < i; j++) styles[j] = STYLE_PREPROCESSOR;
            }
            continue;
        }

        i++;
    }
}