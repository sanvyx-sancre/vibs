#include "syntax.h"
#include "config.h"
#include <ctype.h>
#include <ncurses.h>
#include <string.h>

static int syntax_colors_ready = 0;
static syntax_mode_t current_mode = SYNTAX_MODE_C;
syntax_mode_t syntax_detect_mode(const char *filename) {
    if (!filename) return SYNTAX_MODE_C;
    const char *dot = strrchr(filename, '.');
    if (!dot) return SYNTAX_MODE_C;
    if (strcmp(dot, ".py") == 0) return SYNTAX_MODE_PYTHON;
    return SYNTAX_MODE_C;
}

// per-language builders (implemented in separate files)
void syntax_c_build_styles(const char *line, syntax_style_t *styles, int len);
void syntax_python_build_styles(const char *line, syntax_style_t *styles, int len);

// Switch the active syntax mode.
void syntax_set_mode(syntax_mode_t mode) {
    current_mode = mode;
}

syntax_mode_t syntax_get_mode(void) {
    return current_mode;
}

// Set up ncurses color pairs for the syntax styles.
void syntax_init_colors(void) {
    if (syntax_colors_ready) return;
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, syntax_color_for("keyword"), -1);
        init_pair(2, syntax_color_for("string"), -1);
        init_pair(3, syntax_color_for("comment"), -1);
        init_pair(4, syntax_color_for("number"), -1);
        init_pair(5, syntax_color_for("preprocessor"), -1);
    }
    syntax_colors_ready = 1;
}

// Build a simple style map for the current line.
void syntax_build_styles(const char *line, syntax_style_t *styles, int len) {
    if (!syntax_highlighting_enabled()) {
        for (int i = 0; i < len; i++) {
            styles[i] = STYLE_NORMAL;
        }
        return;
    }

    // delegate to the language-specific builder
    if (current_mode == SYNTAX_MODE_PYTHON) {
        syntax_python_build_styles(line, styles, len);
    } else {
        syntax_c_build_styles(line, styles, len);
    }
}
