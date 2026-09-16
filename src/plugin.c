#include "plugin.h"
#include "buffer.h"
#include <stdio.h>
#include <string.h>

// keep plugin registrations bounded and easy to inspect
#define MAX_PLUGIN_COMMANDS 128
#define MAX_PLUGIN_EVENTS 128

typedef struct {
    char name[64];
    vibs_command_fn callback;
    void *userdata;
} command_entry_t;

typedef struct {
    vibs_event_t event;
    vibs_event_fn callback;
    void *userdata;
} event_entry_t;

static command_entry_t commands[MAX_PLUGIN_COMMANDS];
static size_t command_count;
static event_entry_t events[MAX_PLUGIN_EVENTS];
static size_t event_count;

// register a unique command name
int vibs_register_command(const char *name, vibs_command_fn callback, void *userdata) {
    if (!name || !name[0] || !callback || command_count >= MAX_PLUGIN_COMMANDS) return 0;
    if (strlen(name) >= sizeof(commands[0].name)) return 0;
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(commands[i].name, name) == 0) return 0;
    }
    strcpy(commands[command_count].name, name);
    commands[command_count].callback = callback;
    commands[command_count].userdata = userdata;
    command_count++;
    return 1;
}

// register a callback for an editor event
int vibs_register_event(vibs_event_t event, vibs_event_fn callback, void *userdata) {
    if (!callback || event_count >= MAX_PLUGIN_EVENTS) return 0;
    events[event_count].event = event;
    events[event_count].callback = callback;
    events[event_count].userdata = userdata;
    event_count++;
    return 1;
}

// split a command into its name and argument string
int vibs_execute_command(const char *command, char *message, size_t message_size) {
    if (!command) return 0;
    while (*command == ' ') command++;
    const char *args = strchr(command, ' ');
    size_t name_length = args ? (size_t)(args - command) : strlen(command);
    if (name_length == 0 || name_length >= sizeof(commands[0].name)) return 0;

    for (size_t i = 0; i < command_count; i++) {
        if (strlen(commands[i].name) == name_length && strncmp(commands[i].name, command, name_length) == 0) {
            while (args && *args == ' ') args++;
            if (message && message_size > 0) message[0] = '\0';
            commands[i].callback(args ? args : "", message, message_size, commands[i].userdata);
            return 1;
        }
    }
    return 0;
}

// notify every callback registered for this event
void vibs_emit_event(vibs_event_t event) {
    for (size_t i = 0; i < event_count; i++) {
        if (events[i].event == event) events[i].callback(event, events[i].userdata);
    }
}

// clear registrations when the editor exits
void vibs_shutdown_plugins(void) {
    vibs_emit_event(VIBS_EVENT_SHUTDOWN);
    command_count = 0;
    event_count = 0;
}

// expose the used portion of the buffer
size_t vibs_line_count(void) {
    size_t count = 0;
    for (size_t i = 0; i < MAX_LINES - 1; i++) {
        if (buffer[i][0] != '\0') count = i + 1;
    }
    return count;
}

const char *vibs_get_line(size_t line) {
    if (line >= MAX_LINES - 1) return NULL;
    return buffer[line];
}

int vibs_set_line(size_t line, const char *text) {
    if (line >= MAX_LINES - 1 || !text || strlen(text) >= MAX_COLS) return 0;
    buffer_history_begin();
    strcpy(buffer[line], text);
    buffer_history_end();
    return 1;
}

void vibs_get_cursor(int *line, int *column) {
    if (line) *line = cy;
    if (column) *column = cx;
}

int vibs_set_cursor(int line, int column) {
    if (line < 0 || line >= MAX_LINES - 1 || column < 0 || column > (int)strlen(buffer[line])) return 0;
    cy = line;
    cx = column;
    return 1;
}

const char *vibs_current_file(void) {
    return filename;
}
