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
    "command_mode = \":\"\n";

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

// Dummy color store, replace with real logic
static int colors[8];

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
    // Placeholder until TOML color loading is added
    return COLOR_WHITE;
}
