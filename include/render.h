#ifndef RENDER_H
#define RENDER_H

void draw(void);
extern int insert_mode;
extern int pending_command;
extern int visual_mode;
extern int screen_top;
extern int cx, cy;
extern char *filename;

#endif