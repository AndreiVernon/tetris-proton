#ifndef __ASSETS_H__
#define __ASSETS_H__

#include "pico/stdlib.h"

#define V 0.85
#define FULL_WHITE {255, 255, 255}
#define DEFAULT_WHITE {255*V, 255*V, 255*V}
#define WARM_WHITE {249*V, 255*V, 178*V}
#define BLACK {0, 0, 0}
#define DARK_GREY {63, 63, 63}
#define DARK_BLUE {0, 0, 255}
#define LIGHT_BLUE {0, 255, 255}
#define RED {255, 0, 0}
#define ORANGE {255, 170, 0}
#define YELLOW {255, 255, 0}
#define PURPLE {153, 0, 255}
#define GREEN {0, 255, 0}
#define RUST {119, 50, 50}

#define K BLACK
#define Y DEFAULT_WHITE

uint8_t background[64][64][3] = {
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, Y, K, Y, Y, Y, K, Y, K, K, K, Y, Y, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, Y, K, Y, K, Y, K, Y, K, K, K, Y, K, Y, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, Y, Y, K, Y, K, Y, K, Y, K, K, K, Y, K, Y, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, Y, K, Y, Y, Y, K, Y, Y, Y, K, Y, Y, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, Y, K, K, Y, K, Y, Y, Y, K, Y, K, K, Y, K, Y, Y, Y, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, Y, Y, K, Y, K, Y, Y, K, K, K, Y, K, K, K, K, Y, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, Y, K, Y, Y, K, Y, K, K, K, K, K, Y, K, K, K, Y, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, Y, K, K, Y, K, Y, Y, Y, K, Y, K, K, Y, K, K, Y, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
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
};

#endif