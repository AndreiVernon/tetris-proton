#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "pico/stdlib.h"

#define TARGET_FRAMERATE 90

extern volatile bool frame_ready;   //frame ready to display

void render_frame(bool swap_fbf);
void render_title();
void init_frame_timer();
void draw_text(const char *s, int x, int y, bool center, uint8_t shape_id);
void fadeout(int duration_ms);
void fadein(int duration_ms);

#endif