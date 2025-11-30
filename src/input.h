#ifndef __INPUT_H__
#define __INPUT_H__

typedef struct {
    bool left, right;
    bool rot_left, rot_right;
    bool soft_drop, hard_drop;
    bool hold, pause;
} InputState;


extern InputState cur_inputs;

void init_inputs();
void get_inputs();

#endif