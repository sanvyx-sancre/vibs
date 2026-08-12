#include "buffer.h"   // brings in MAX_LINES, MAX_COLS, filename, buffer, cx, cy
#include <stdio.h>    // FILE, fopen, fclose, snprintf
#include <string.h>   // strlen, strcspn, memcpy
#include <stdlib.h>   // malloc, free

char buffer[MAX_LINES][MAX_COLS];
char *filename = NULL;
int cx = 0, cy = 0;

typedef struct {
    char lines[MAX_LINES][MAX_COLS];
    int cx;
    int cy;
} buffer_snapshot_t;

#define MAX_HISTORY 200
static buffer_snapshot_t undo_stack[MAX_HISTORY];
static buffer_snapshot_t redo_stack[MAX_HISTORY];
static int undo_count = 0;
static int redo_count = 0;
static int history_open = 0;

static void snapshot_current(buffer_snapshot_t *snap) {
    memcpy(snap->lines, buffer, sizeof(buffer));
    snap->cx = cx;
    snap->cy = cy;
}

static void restore_snapshot(const buffer_snapshot_t *snap) {
    memcpy(buffer, snap->lines, sizeof(buffer));
    cx = snap->cx;
    cy = snap->cy;
}

static char *my_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *p = malloc(len);
    if (p) memcpy(p, s, len);
    return p;
}

void load_file(const char* fname) {
    FILE *f = fopen(fname, "r");
    if (!f) {
        // Try to create the file if it doesn't exist
        f = fopen(fname, "a");
        if (!f) {
            // Still can’t open/create
            for (int i = 0; i < MAX_LINES; i++)
                buffer[i][0] = '\0';
            snprintf(buffer[0], MAX_COLS, "Error: could not open or create file '%s'", fname);
            if (filename) {
                free(filename);
                filename = NULL;
            }
            return;
        }
        fclose(f);
        // Now open for reading
        f = fopen(fname, "r");
        if (!f) {
            for (int i = 0; i < MAX_LINES; i++)
                buffer[i][0] = '\0';
            snprintf(buffer[0], MAX_COLS, "Error: could not open file '%s' after creation", fname);
            if (filename) {
                free(filename);
                filename = NULL;
            }
            return;
        }
    }

    // Debug: free old filename if any
    if (filename) {
        printf("[DEBUG] Freeing filename ptr %p: '%s'\n", (void*)filename, filename);
        free(filename);
        filename = NULL;
    }

    // Store new filename
    filename = my_strdup(fname);
    printf("[DEBUG] Allocated filename ptr %p: '%s'\n", (void*)filename, filename);

    // Read lines into buffer
    int i = 0;
    while (fgets(buffer[i], MAX_COLS, f) && i < MAX_LINES - 1) {
        buffer[i][strcspn(buffer[i], "\n")] = '\0';
        i++;
    }
    // Clear remaining lines
    for (; i < MAX_LINES; i++) {
        buffer[i][0] = '\0';
    }

    fclose(f);
    cx = cy = 0;
}

void save_file(void) {
    if (!filename) {
        printf("[ERROR] No filename set, cannot save.\n");
        return;
    }

    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("[ERROR] Could not open file '%s' for writing.\n", filename);
        return;
    }

    for (int i = 0; i < MAX_LINES - 1; i++) {
        if (strlen(buffer[i]) > 0)
            fprintf(f, "%s\n", buffer[i]);
    }
    fclose(f);
}

void buffer_history_push(void) {
    if (undo_count >= MAX_HISTORY) {
        for (int i = 1; i < MAX_HISTORY; i++) {
            undo_stack[i - 1] = undo_stack[i];
        }
        undo_count = MAX_HISTORY - 1;
    }
    snapshot_current(&undo_stack[undo_count]);
    undo_count++;
    redo_count = 0;
}

void buffer_history_begin(void) {
    if (!history_open) {
        buffer_history_push();
        history_open = 1;
    }
}

void buffer_history_end(void) {
    history_open = 0;
}

void buffer_history_undo(void) {
    if (undo_count <= 0) return;
    undo_count--;
    if (redo_count >= MAX_HISTORY) {
        for (int i = 1; i < MAX_HISTORY; i++) {
            redo_stack[i - 1] = redo_stack[i];
        }
        redo_count = MAX_HISTORY - 1;
    }
    buffer_snapshot_t current;
    snapshot_current(&current);
    redo_stack[redo_count++] = current;
    restore_snapshot(&undo_stack[undo_count]);
}

void buffer_history_redo(void) {
    if (redo_count <= 0) return;
    redo_count--;
    if (undo_count >= MAX_HISTORY) {
        for (int i = 1; i < MAX_HISTORY; i++) {
            undo_stack[i - 1] = undo_stack[i];
        }
        undo_count = MAX_HISTORY - 1;
    }
    buffer_snapshot_t current;
    snapshot_current(&current);
    undo_stack[undo_count++] = current;
    restore_snapshot(&redo_stack[redo_count]);
}
