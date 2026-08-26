#include <syntax.h>
#include <ctype.h>
#include <string.h>

void syntax_go_build_styles(const char *line, syntax_style_t *styles, int len) {
    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;
    if (len > 0 && line[0] == '#') { for (int j = 0; j < len; j++) styles[j] = STYLE_COMMENT; return; }
}