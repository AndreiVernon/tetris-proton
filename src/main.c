#include "pico/multicore.h"
#include "display.h"
#include "input.h"
#include "tetris.h"
#include "graphics.h"
#include "sound.h"
#include "multiplayer.h"

void main_menu_loop() {
    render_frame();

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
    }
}

int main() {
    stdio_init_all();
    display_init();
    init_inputs();
    init_audio();
    mp_uart_init();

    multicore_launch_core1(display_loop);
    init_frame_timer();

    main_menu_loop();

    game_loop();

    //keep rendering last frame forever
    //replace this with menu system
    while (1) {
        render_frame();
        while (!frame_ready) tight_loop_contents();
        frame_ready = false;
    }

    return 0;
}

void gen_menu_loop() {

    //declare variables: which menu item is selected

    //while game not started
        //get inputs
        //based on input change the selected audio
        //
}