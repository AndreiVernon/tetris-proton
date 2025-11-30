#include "pico/multicore.h"
#include "display.h"
#include "input.h"
#include "tetris.h"
#include "graphics.h"

int main() {
    stdio_init_all();
    display_init();
    init_inputs();

    multicore_launch_core1(display_loop);
    init_frame_timer();

    game_loop();

    return 0;
}