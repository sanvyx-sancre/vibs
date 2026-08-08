#ifndef CONFIG_H
#define CONFIG_H

void load_config(const char *path);
char get_keybinding(const char *action);  // returns char now
int get_color(const char *element);
int show_line_numbers(void);
int syntax_highlighting_enabled(void);
int syntax_color_for(const char *style_name);

#endif
