#ifndef __INPUT_H__
#define __INPUT_H__

typedef struct {
    bool up, down, left, right;
    bool a, b;
    bool select, start;
} RawInputState;

typedef struct {
    bool left, right;
    bool rot_left, rot_right;
    bool soft_drop, hard_drop;
    bool hold, pause;

    bool up, down, left_edge, right_edge, a, b, select, start; //for menus
} InputState;

extern RawInputState raw_inputs;
extern InputState cur_inputs;

void init_inputs();
void get_inputs();

#endif