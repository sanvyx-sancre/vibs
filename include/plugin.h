#ifndef PLUGIN_H
#define PLUGIN_H

#include <stddef.h>

// events exposed to plugins
typedef enum {
    VIBS_EVENT_STARTUP,
    VIBS_EVENT_FILE_OPEN,
    VIBS_EVENT_BEFORE_SAVE,
    VIBS_EVENT_AFTER_SAVE,
    VIBS_EVENT_TEXT_CHANGED,
    VIBS_EVENT_CURSOR_MOVE,
    VIBS_EVENT_SHUTDOWN
} vibs_event_t;

// callbacks receive editor data through a small stable interface
typedef int (*vibs_command_fn)(const char *args, char *message, size_t message_size, void *userdata);
typedef void (*vibs_event_fn)(vibs_event_t event, void *userdata);

// command and event registration
int vibs_register_command(const char *name, vibs_command_fn callback, void *userdata);
int vibs_register_event(vibs_event_t event, vibs_event_fn callback, void *userdata);
int vibs_execute_command(const char *command, char *message, size_t message_size);
void vibs_emit_event(vibs_event_t event);
void vibs_shutdown_plugins(void);

// editor state exposed to plugins
size_t vibs_line_count(void);
const char *vibs_get_line(size_t line);
int vibs_set_line(size_t line, const char *text);
void vibs_get_cursor(int *line, int *column);
int vibs_set_cursor(int line, int column);
const char *vibs_current_file(void);
int vibs_python_load(const char *plugin_dir);
const char *vibs_python_status(void);

#endif
