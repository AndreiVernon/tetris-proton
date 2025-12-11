#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "pico/stdlib.h"

//#define TARGET_FRAMERATE 120

extern int TARGET_FRAMERATE;

extern volatile bool frame_ready;   //frame ready to display

void init_frame_timer();
void wait_and_push_frame();
void get_target_framerate();

void render_frame();
void render_main_menu(int cur_sel, bool mp_wait);
void render_options(int cur_sel);
void render_game_over(int cur_sel, bool mp_wait, bool game_was_mp);
void draw_text(const char *s, int x, int y, int justify, uint8_t shape_id);
void fade(int duration_ms, bool dir);

#endif