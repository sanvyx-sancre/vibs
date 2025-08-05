#include "buffer.h"   // brings in MAX_LINES, MAX_COLS, filename, buffer, cx, cy
#include <stdio.h>    // for FILE, fopen, fclose, etc
#include <string.h>   // for strcspn

char buffer[MAX_LINES][MAX_COLS];
char *filename = NULL;
int cx = 0, cy = 0;

void load_file(const char* fname) {
    FILE *f = fopen(fname, "r");
    if (!f) return;

    int i = 0;
    while (fgets(buffer[i], MAX_COLS, f) && i < MAX_LINES - 1) {
        buffer[i][strcspn(buffer[i], "\n")] = '\0';
        i++;
    }
    fclose(f);
}

void save_file(void) {
    FILE *f = fopen(filename, "w");
    if (!f) return;

    for (int i = 0; i < MAX_LINES - 1; i++) {
        if (strlen(buffer[i]) > 0)
            fprintf(f, "%s\n", buffer[i]);
    }
    fclose(f);
}