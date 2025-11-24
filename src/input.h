#ifndef __INPUT_H__
#define __INPUT_H__

typedef struct {
    bool up, down, left, right;
    bool a, b;
    bool select, start;
} InputState;

void read_input();

#endif