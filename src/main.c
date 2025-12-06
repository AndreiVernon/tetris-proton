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
            break;
        }

        if (cur_inputs.pause) {
            multiplayer = true;
            mp_test_en = true;
            break;
        }

        if (cur_inputs.right) {
            multiplayer = false;
            break;
        }
    }

    if (multiplayer) {
        mp_uart_init();

        if (mp_test_en) return;

        //detect other console
        while (!mp_handshake_blocking(1000)) {
            tight_loop_contents();
        }

        //sync with other console
        while (!mp_sync_ready) {
            mp_send_msg_packed(mp_msg_ping, 2);
            sleep_us(20);
        }
    }
}

int main() {
    stdio_init_all();
    display_init();
    init_inputs();
    init_audio();
    //mp_uart_init();

    multicore_launch_core1(display_loop);
    init_frame_timer();

    init_game_blank();

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