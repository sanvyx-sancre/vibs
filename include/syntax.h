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

typedef enum {
    SYNTAX_MODE_PLAIN,
    SYNTAX_MODE_C,
    SYNTAX_MODE_CPP,
    SYNTAX_MODE_PYTHON,
    SYNTAX_MODE_LUA
} syntax_mode_t;

void syntax_init_colors(void);
void syntax_set_mode(syntax_mode_t mode);
syntax_mode_t syntax_get_mode(void);
syntax_mode_t syntax_detect_mode(const char *filename);
void syntax_build_styles(const char *line, syntax_style_t *styles, int len);

#endif
