#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

#define MAX_LINES 1024
#define MAX_COLS 1024

extern char buffer[MAX_LINES][MAX_COLS];
extern char *filename;
extern int cx, cy;

void load_file(const char* fname);
void save_file(void);
void buffer_history_push(void);
void buffer_history_begin(void);
void buffer_history_end(void);
void buffer_history_undo(void);
void buffer_history_redo(void);

#endif
