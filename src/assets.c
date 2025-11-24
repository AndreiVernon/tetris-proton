#include "pico/stdlib.h"
#include "assets.h"

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
    {K, K, K, K, K, K, K, K, K, K, K, K, K, WW, WW, WW, K, WW, WW, WW, K, K, K, WW, WW, WW, K, WW, WW, WW, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, WW, K, WW, K, K, WW, K, K, WW, K, WW, K, K, K, K, K, WW, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, WW, K, WW, K, K, WW, K, K, K, K, WW, WW, WW, K, K, WW, WW, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, R, R, R, R, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, WW, K, WW, K, WW, WW, K, K, WW, K, K, K, WW, K, K, K, WW, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, R, R, R, R, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, WW, WW, WW, K, K, WW, K, K, K, K, WW, WW, WW, K, WW, WW, WW, K, K, K, K, K, K, K, K, K, K, K, K, K, K, R, R, R, R, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, R, R, R, R, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, W, K, W, W, W, K, W, W, W, K, K, K, W, K, W, W, W, K, W, W, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, W, K, W, K, W, K, K, K, W, K, W, K, W, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, O, O, O, O, O, O, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, W, W, W, K, W, W, W, K, W, W, W, K, K, W, W, K, W, W, W, K, W, W, W, K, K, K, K, K, K, K, K, K, K, K, K, O, O, O, O, O, O, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, W, K, W, K, W, K, K, K, W, K, K, K, K, K, W, K, W, K, W, K, W, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, O, O, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, W, K, W, K, W, W, W, K, W, W, W, K, W, W, W, K, W, W, W, K, W, W, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, O, O, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, K, K, K, K, K, K, K, K, K, K, K, K, GR, GR, GR, GR, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, GR, GR, GR, GR, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, GR, GR, GR, GR, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, GR, GR, GR, GR, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, DB, DB, DB, DB, DB, DB, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, DB, DB, DB, DB, DB, DB, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, DB, DB, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, DB, DB, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, P, P, P, P, P, P, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, P, P, P, P, P, P, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, P, P, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, P, P, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, Y, Y, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, Y, Y, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, Y, Y, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, Y, Y, Y, Y, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, LB, LB, LB, LB, LB, LB, LB, LB, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, LB, LB, LB, LB, LB, LB, LB, LB, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, W, K, K, W, K, W, W, W, K, W, K, K, W, K, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, W, K, W, W, K, W, K, K, K, K, K, W, K, K, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, W, W, K, W, K, W, W, K, K, K, W, K, K, K, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, W, K, K, W, K, W, W, W, K, W, K, K, W, K, W, W, W, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, GR, GR, GR, GR, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, GR, GR, GR, GR, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, GR, GR, GR, GR, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, GR, GR, GR, GR, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, K, W, K, W, W, W, K, W, W, W, K, W, W, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, W, K, W, K, W, K, W, K, W, K, K, K, W, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, W, W, W, K, W, K, W, K, W, K, K, K, W, K, W, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, W, K, W, K, W, W, W, K, W, K, K, K, W, W, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
    {K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, DGY, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K},
};