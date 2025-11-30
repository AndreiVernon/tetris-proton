#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "pico/stdlib.h"

//panel dimensions
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANEL_ROWS 32 //# of row addresses 
//(scans 32 times to cover all 64 physical rows by scanning upper half and lower half)

//framebuffer[id][row][col][rgb] - 3d array that stores what to display
extern uint8_t framebuffer[2][PANEL_HEIGHT][PANEL_WIDTH][3];
//which framebuffer is ready to present
extern volatile bool fbf_rdy;

void display_init();
void display_clear();
void display_loop();

#endif