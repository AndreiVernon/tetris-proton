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
#define DARK_RED {100, 22, 22}
//garbage
#define GREY {50, 35, 35}


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
#define DR DARK_RED
#define GY GREY

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
    {K, K, K, K, K, K, K, K, K, K, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, K, K, K, K, K, K, K, K, W, K, W, K, W, W, W, K, W, W, W, K, W, W, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, W, K, K, K, K, K, K, K, K, W, K, W, K, W, K, W, K, W, K, K, K, W, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, K, K, K, K, K, K, K, K, W, W, W, K, W, K, W, K, W, K, K, K, W, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, W, K, K, K, K, K, K, K, K, W, K, W, K, W, W, W, K, W, K, K, K, W, W, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
};

const bool font_mask[38][5][5] = {
    {{1, 1, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 1, 1}}, //0
    {{1, 1, 1}, {0, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 1, 0}}, //1
    {{1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1}}, //2
    {{1, 1, 1}, {0, 0, 1}, {0, 1, 1}, {0, 0, 1}, {1, 1, 1}}, //3
    {{0, 0, 1}, {0, 0, 1}, {1, 1, 1}, {1, 0, 1}, {1, 0, 1}}, //4
    {{1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1}}, //5
    {{1, 1, 1}, {1, 0, 1}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1}}, //6
    {{0, 0, 1}, {0, 0, 1}, {0, 1, 1}, {0, 0, 1}, {1, 1, 1}}, //7
    {{1, 1, 1}, {1, 0, 1}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1}}, //8
    {{1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1}}, //9

    {{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 1, 1}, {1, 0, 0, 1}, {0, 1, 1, 0}}, //A
    {{1, 1, 1, 0}, {1, 0, 0, 1}, {1, 1, 1, 0}, {1, 0, 0, 1}, {1, 1, 1, 0}}, //B
    {{0, 1, 1, 0}, {1, 0, 0, 1}, {1, 0, 0, 0}, {1, 0, 0, 1}, {0, 1, 1, 0}}, //C
    {{1, 1, 1, 0}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 1, 0}}, //D
    {{1, 1, 1, 1}, {1, 0, 0, 0}, {1, 1, 1, 1}, {1, 0, 0, 0}, {1, 1, 1, 1}}, //E
    {{1, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 1}}, //F
    {{0, 1, 1, 1}, {1, 0, 0, 1}, {1, 0, 1, 1}, {1, 0, 0, 0}, {0, 1, 1, 1}}, //G
    {{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 1, 1}, {1, 0, 0, 1}, {1, 1, 1, 1}}, //H
    {{1, 1, 1}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {1, 1, 1}}, //I
    {{0, 1, 1, 0}, {1, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 1, 1}}, //J
    {{1, 0, 0, 1}, {1, 0, 1, 0}, {1, 1, 0, 0}, {1, 0, 1, 0}, {1, 0, 0, 1}}, //K
    {{1, 1, 1}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}}, //L 
    {{1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 1, 0, 1}, {1, 1, 0, 1, 1}, {1, 0, 0, 0, 1}}, //M
    {{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 1, 1}, {1, 1, 0, 1}, {1, 0, 0, 1}}, //N
    {{1, 1, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 1, 1}}, //O
    {{1, 0, 0}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1}}, //P    
    {{0, 1, 0, 1}, {1, 0, 1, 0}, {1, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 1, 0}}, //Q
    {{1, 0, 1}, {1, 0, 1}, {1, 1, 0}, {1, 1, 0}, {1, 1, 0}}, //R
    {{1, 1, 0}, {0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {0, 1, 1}}, //S
    {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {1, 1, 1}}, //T
    {{1, 1, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}}, //U
    {{0, 1, 0}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}}, //V
    {{0, 1, 0, 1, 0}, {1, 0, 1, 0, 1}, {1, 0, 1, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}}, //W
    {{1, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 1, 0}, {1, 0, 0, 1}, {1, 0, 0, 1}}, //X
    {{0, 1, 0}, {0, 1, 0}, {1, 1, 1}, {1, 0, 1}, {1, 0, 1}}, //Y
    {{1, 1, 1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}}, //Z

    
    {{0, 0, 0}, {0, 1, 0}, {0, 0, 0}, {0, 1, 0}, {0, 0, 0}}, // :
    {{1}, {0}, {1}, {1}, {1}} //!
};

#endif