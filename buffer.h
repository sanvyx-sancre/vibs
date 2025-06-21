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

#endif
