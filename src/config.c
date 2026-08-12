#include "config.h"
#include "toml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <sys/stat.h>
#include <errno.h>

static void ensure_config_dir(const char *path) {
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error creating config directory: %s\n", strerror(errno));
        exit(1);
    }
}

static void create_default_config(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "Failed to create config file %s: %s\n", path, strerror(errno));
        exit(1);
    }
    // Minimal default TOML config (change as you want)
    const char *default_toml = 
    "# Default vibs config\n"
    "[keys]\n"
    "insert_mode = \"i\"\n"
    "append_mode = \"a\"\n"
    "visual_mode = \"v\"\n"
    "delete = \"d\"\n"
    "move_left = \"h\"\n"
    "move_right = \"l\"\n"
    "move_up = \"k\"\n"
    "move_down = \"j\"\n"
    "command_mode = \":\"\n"
    "undo = \"u\"\n"
    "redo = \"r\"\n"
    "\n"
    "[ui]\n"
    "show_line_numbers = true\n"
    "syntax_highlighting = true\n"
    "keyword_color = \"cyan\"\n"
    "string_color = \"green\"\n"
    "comment_color = \"magenta\"\n"
    "number_color = \"yellow\"\n"
    "preprocessor_color = \"blue\"\n";

    fprintf(f, "%s", default_toml);
    fclose(f);
    printf("Created default config at %s\n", path);
}

#define MAX_BINDINGS 32

typedef struct {
    char action[32];
    char key; // single char
} KeyBinding;

static KeyBinding keybindings[MAX_BINDINGS];
static int keybinding_count = 0;
static int line_numbers_enabled = 1;
static int syntax_enabled = 1;
static int syntax_keyword_color = COLOR_CYAN;
static int syntax_string_color = COLOR_GREEN;
static int syntax_comment_color = COLOR_MAGENTA;
static int syntax_number_color = COLOR_YELLOW;
static int syntax_preprocessor_color = COLOR_BLUE;

void load_config(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        // Create config dir & default config if missing
        char config_dir[512];
        strncpy(config_dir, path, sizeof(config_dir) - 1);
        config_dir[sizeof(config_dir) - 1] = 0;
        char *last_slash = strrchr(config_dir, '/');
        if (last_slash) *last_slash = 0;

        ensure_config_dir(config_dir);
        create_default_config(path);
        return;
    }

    char errbuf[200];
    toml_table_t *conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) {
        fprintf(stderr, "TOML Parse Error: %s\n", errbuf);
        return;
    }

    toml_table_t *keys = toml_table_in(conf, "keys");
    if (keys) {
        for (int i = 0; i < toml_table_nkval(keys); i++) {
            const char *action = toml_key_in(keys, i);
            toml_datum_t val = toml_string_in(keys, action);
            if (val.ok && keybinding_count < MAX_BINDINGS) {
                strncpy(keybindings[keybinding_count].action, action, 31);
                keybindings[keybinding_count].action[31] = '\0';
                keybindings[keybinding_count].key = val.u.s[0]; // first char only
                keybinding_count++;
                free(val.u.s);
            }
        }
    }

    toml_table_t *ui = toml_table_in(conf, "ui");
    if (ui) {
        toml_datum_t show_numbers = toml_bool_in(ui, "show_line_numbers");
        if (show_numbers.ok) {
            line_numbers_enabled = show_numbers.u.b;
        }

        toml_datum_t syntax_enabled_flag = toml_bool_in(ui, "syntax_highlighting");
        if (syntax_enabled_flag.ok) {
            syntax_enabled = syntax_enabled_flag.u.b;
        }

        toml_datum_t keyword_color = toml_string_in(ui, "keyword_color");
        if (keyword_color.ok) {
            if (strcmp(keyword_color.u.s, "cyan") == 0) syntax_keyword_color = COLOR_CYAN;
            else if (strcmp(keyword_color.u.s, "green") == 0) syntax_keyword_color = COLOR_GREEN;
            else if (strcmp(keyword_color.u.s, "magenta") == 0) syntax_keyword_color = COLOR_MAGENTA;
            else if (strcmp(keyword_color.u.s, "yellow") == 0) syntax_keyword_color = COLOR_YELLOW;
            else if (strcmp(keyword_color.u.s, "blue") == 0) syntax_keyword_color = COLOR_BLUE;
            else if (strcmp(keyword_color.u.s, "red") == 0) syntax_keyword_color = COLOR_RED;
            free(keyword_color.u.s);
        }

        toml_datum_t string_color = toml_string_in(ui, "string_color");
        if (string_color.ok) {
            if (strcmp(string_color.u.s, "cyan") == 0) syntax_string_color = COLOR_CYAN;
            else if (strcmp(string_color.u.s, "green") == 0) syntax_string_color = COLOR_GREEN;
            else if (strcmp(string_color.u.s, "magenta") == 0) syntax_string_color = COLOR_MAGENTA;
            else if (strcmp(string_color.u.s, "yellow") == 0) syntax_string_color = COLOR_YELLOW;
            else if (strcmp(string_color.u.s, "blue") == 0) syntax_string_color = COLOR_BLUE;
            else if (strcmp(string_color.u.s, "red") == 0) syntax_string_color = COLOR_RED;
            free(string_color.u.s);
        }

        toml_datum_t comment_color = toml_string_in(ui, "comment_color");
        if (comment_color.ok) {
            if (strcmp(comment_color.u.s, "cyan") == 0) syntax_comment_color = COLOR_CYAN;
            else if (strcmp(comment_color.u.s, "green") == 0) syntax_comment_color = COLOR_GREEN;
            else if (strcmp(comment_color.u.s, "magenta") == 0) syntax_comment_color = COLOR_MAGENTA;
            else if (strcmp(comment_color.u.s, "yellow") == 0) syntax_comment_color = COLOR_YELLOW;
            else if (strcmp(comment_color.u.s, "blue") == 0) syntax_comment_color = COLOR_BLUE;
            else if (strcmp(comment_color.u.s, "red") == 0) syntax_comment_color = COLOR_RED;
            free(comment_color.u.s);
        }

        toml_datum_t number_color = toml_string_in(ui, "number_color");
        if (number_color.ok) {
            if (strcmp(number_color.u.s, "cyan") == 0) syntax_number_color = COLOR_CYAN;
            else if (strcmp(number_color.u.s, "green") == 0) syntax_number_color = COLOR_GREEN;
            else if (strcmp(number_color.u.s, "magenta") == 0) syntax_number_color = COLOR_MAGENTA;
            else if (strcmp(number_color.u.s, "yellow") == 0) syntax_number_color = COLOR_YELLOW;
            else if (strcmp(number_color.u.s, "blue") == 0) syntax_number_color = COLOR_BLUE;
            else if (strcmp(number_color.u.s, "red") == 0) syntax_number_color = COLOR_RED;
            free(number_color.u.s);
        }

        toml_datum_t preprocessor_color = toml_string_in(ui, "preprocessor_color");
        if (preprocessor_color.ok) {
            if (strcmp(preprocessor_color.u.s, "cyan") == 0) syntax_preprocessor_color = COLOR_CYAN;
            else if (strcmp(preprocessor_color.u.s, "green") == 0) syntax_preprocessor_color = COLOR_GREEN;
            else if (strcmp(preprocessor_color.u.s, "magenta") == 0) syntax_preprocessor_color = COLOR_MAGENTA;
            else if (strcmp(preprocessor_color.u.s, "yellow") == 0) syntax_preprocessor_color = COLOR_YELLOW;
            else if (strcmp(preprocessor_color.u.s, "blue") == 0) syntax_preprocessor_color = COLOR_BLUE;
            else if (strcmp(preprocessor_color.u.s, "red") == 0) syntax_preprocessor_color = COLOR_RED;
            free(preprocessor_color.u.s);
        }
    }

    toml_free(conf);
}

char get_keybinding(const char *action) {
    for (int i = 0; i < keybinding_count; i++) {
        if (strcmp(keybindings[i].action, action) == 0) {
            return keybindings[i].key;
        }
    }
    return '\0';
}

int get_color(const char *element) {
    (void)element;
    return COLOR_WHITE;
}

int show_line_numbers(void) {
    return line_numbers_enabled;
}

int syntax_highlighting_enabled(void) {
    return syntax_enabled;
}

int syntax_color_for(const char *style_name) {
    if (strcmp(style_name, "keyword") == 0) return syntax_keyword_color;
    if (strcmp(style_name, "string") == 0) return syntax_string_color;
    if (strcmp(style_name, "comment") == 0) return syntax_comment_color;
    if (strcmp(style_name, "number") == 0) return syntax_number_color;
    if (strcmp(style_name, "preprocessor") == 0) return syntax_preprocessor_color;
    return COLOR_WHITE;
}
