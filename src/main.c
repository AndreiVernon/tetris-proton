#include "pico/multicore.h"
#include "main.h"
#include "display.h"
#include "input.h"
#include "tetris.h"
#include "graphics.h"
#include "sound.h"
#include "multiplayer.h"

screen_t cur_screen;

void main_menu_loop() {
    render_title();

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

    play_audio(SILENCE_SONG, false);
    fadeout(250);

    cur_screen = game_screen;
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

    cur_screen = title_screen;

    while (1) {
        switch (cur_screen) {
            case title_screen:
                main_menu_loop();
                break;
            case game_screen:
                game_loop();
                break;
        }
    }

    return 0;
}