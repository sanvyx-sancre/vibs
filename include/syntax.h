#ifndef SYNTAX_H
#define SYNTAX_H

typedef enum {
    STYLE_NORMAL,
    STYLE_KEYWORD,
    STYLE_STRING,
    STYLE_COMMENT,
    STYLE_NUMBER,
    STYLE_PREPROCESSOR
} syntax_style_t;

void syntax_init_colors(void);
void syntax_build_styles(const char *line, syntax_style_t *styles, int len);

#endif
