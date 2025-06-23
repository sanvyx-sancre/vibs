#ifndef CONFIG_H
#define CONFIG_H

void load_config(const char *path);
char get_keybinding(const char *action);  // returns char now
int get_color(const char *element);

#endif
