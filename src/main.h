#ifndef __MAIN_H__
#define __MAIN_H__

typedef enum {
    title_screen,
    options_screen,
    game_screen,
    game_over_screen,
} screen_t;

extern screen_t cur_screen;

#endif