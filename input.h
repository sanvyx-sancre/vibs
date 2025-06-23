#ifndef INPUT_H
#define INPUT_H

void handle_input(int ch);
void load_keys_from_config(void);

extern int visual_mode;
extern int insert_mode; 
extern int pending_command;


#endif
