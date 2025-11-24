#ifndef __ASSETS_H__
#define __ASSETS_H__

#include "pico/stdlib.h"

#define V 1
#define FULL_WHITE {255, 255, 255}
#define DEFAULT_WHITE {255*V, 255*V, 255*V}
#define WARM_WHITE {243, 255, 76}
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

//background mask, from bottom to top
extern const uint8_t background[64][64][3];

#endif