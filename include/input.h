#ifndef INPUT_H
#define INPUT_H

void handle_input(int ch);
void load_keys_from_config(void);
void init_input(void);

extern int visual_mode;
extern int insert_mode; 
extern int pending_command;
extern int screen_top;
extern int status_line_visible;


#endif
