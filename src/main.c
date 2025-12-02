#include "pico/multicore.h"
#include "display.h"
#include "input.h"
#include "tetris.h"
#include "graphics.h"
#include "sound.h"
#include "multiplayer.h"

void main_menu_loop() {
    while (1) {
        get_inputs();

        if (cur_inputs.left) {
            multiplayer = true;
            return;
        }

        if (cur_inputs.right) {
            multiplayer = false;
            return;
        }
        
        while (!frame_ready) tight_loop_contents();
        frame_ready = false;
    }
}

int main() {
    stdio_init_all();
    display_init();
    init_inputs();
    init_audio();

    multicore_launch_core1(display_loop);
    init_frame_timer();

    main_menu_loop();

    if (multiplayer) {
        multiplayer_uart_init();

        //connection test failed
        if (!multiplayer_init_connection(5)) multiplayer = false;
    }

    game_loop();

    return 0;
}