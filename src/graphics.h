#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "pico/stdlib.h"

#define TARGET_FRAMERATE 90

extern volatile bool frame_ready;   //frame ready to display

void render_frame();
void init_frame_timer();

#endif