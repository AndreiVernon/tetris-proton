#ifndef __ASSETS_H__
#define __ASSETS_H__

#include "pico/stdlib.h"

#define V 1
#define FULL_WHITE {255, 255, 255}
#define DEFAULT_WHITE {255*V, 255*V, 255*V}
#define WARM_WHITE {243, 255, 76}
#define BLACK {0, 0, 0}
#define DARK_GREY {30, 30, 30}
#define DARK_BLUE {0, 8, 255}
#define LIGHT_BLUE {0, 255, 255}
#define RED {255, 0, 0}
#define ORANGE {255, 70, 0}
#define YELLOW {255, 220, 0}
#define PURPLE {153, 0, 255}
#define GREEN {0, 255, 0}
#define RUST {119, 50, 50}


#define K BLACK
#define W DEFAULT_WHITE
#define WW WARM_WHITE
#define DGY DARK_GREY
#define DB DARK_BLUE
#define LB LIGHT_BLUE
#define R RED
#define O ORANGE
#define Y YELLOW
#define P PURPLE
#define GR GREEN
#define RU RUST

//background mask, from bottom to top
const uint8_t background[64][64][3] = {
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, W, K, K, W, K, W, W, W, K, W, K, K, W, K, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, W, K, W, W, K, W, K, K, K, K, K, W, K, K, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, W, W, K, W, K, W, W, K, K, K, W, K, K, K, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, W, K, K, W, K, W, W, W, K, W, K, K, W, K, W, W, W, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, W, K, W, W, W, K, W, W, W, K, W, W, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, W, K, W, K, W, K, W, K, W, K, K, K, W, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, W, W, K, W, K, W, K, W, K, K, K, W, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, W, K, W, K, W, W, W, K, W, K, K, K, W, W, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
};

#endif