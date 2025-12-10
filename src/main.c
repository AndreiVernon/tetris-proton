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
    int cur_sel = 0;
    int NUM_OPTS = 3;
    while (1) {
        get_inputs();

        if (cur_inputs.up) {
            cur_sel = (cur_sel - 1 + NUM_OPTS) % NUM_OPTS;
            play_audio(SWITCH_OPTION_SFX, true);
        }

        if (cur_inputs.down) {
            cur_sel = (cur_sel + 1 + NUM_OPTS) % NUM_OPTS;
            play_audio(SWITCH_OPTION_SFX, true);
        }

        if (cur_inputs.a) {
            play_audio(SELECT_OPTION_SFX, true);
            break;
        }

        render_title(false);
        draw_text("singleplayer", 32, 23, true, cur_sel == 0 ? SELECTED_TEXT : UNSELECTED_TEXT);
        draw_text("multiplayer", 32, 15, true, cur_sel == 1 ? SELECTED_TEXT : UNSELECTED_TEXT);
        draw_text("options", 32, 7, true, cur_sel == 2 ? SELECTED_TEXT : UNSELECTED_TEXT);
        swap_framebuffer();
        wait_for_next_frame();
    }

    if (cur_sel == 0) {
        multiplayer = false;
        cur_screen = game_screen;
        play_audio(SILENCE_SONG, false);
    }

    if (cur_sel == 1) {
        multiplayer = true;

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

        cur_screen = game_screen;
        play_audio(SILENCE_SONG, false);
    }

    if (cur_sel == 2) {
        cur_screen = options_screen;
    }

    //fadeout(250);
}

int main() {
    stdio_init_all();
    display_init();
    init_inputs();
    init_audio();
    mp_uart_init();

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
            case game_over_screen:
                break;
            case options_screen:
                main_menu_loop();
                break;
        }
    }

    return 0;
}