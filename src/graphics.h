#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "pico/stdlib.h"

#define TARGET_FRAMERATE 120

extern volatile bool frame_ready;   //frame ready to display

void init_frame_timer();
void wait_and_push_frame();

void render_frame();
void render_title();
void draw_text(const char *s, int x, int y, bool center, uint8_t shape_id);
void fade(int duration_ms, bool dir);

#endif