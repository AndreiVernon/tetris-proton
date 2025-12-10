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
    multiplayer = false;
    mp_sync_awaiting = false;
    mp_sync_ready = false;

    render_title();
    draw_text("singleplayer", 32, 23, true, SELECTED_TEXT);
    draw_text("multiplayer", 32, 15, true, UNSELECTED_TEXT);
    draw_text("options", 32, 7, true, UNSELECTED_TEXT);
    fade(1000, 1);

    int cur_sel = 0;
    int NUM_OPTS = 3;
    while (1) {
        main_menu_loop_start:
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
            break;
        }

        render_title();
        draw_text("singleplayer", 32, 23, true, cur_sel == 0 ? SELECTED_TEXT : UNSELECTED_TEXT);
        draw_text("multiplayer", 32, 15, true, cur_sel == 1 ? SELECTED_TEXT : UNSELECTED_TEXT);
        draw_text("options", 32, 7, true, cur_sel == 2 ? SELECTED_TEXT : UNSELECTED_TEXT);
        wait_and_push_frame();
    }

    if (cur_sel == 0) {
        multiplayer = false;
        cur_screen = game_screen;
        play_audio(SELECT_OPTION_SFX, true);
        play_audio(SILENCE_SONG, false);
    }

    if (cur_sel == 1) {
        //detect other console
        if (mp_handshake_blocking(1000)) {
            play_audio(SELECT_OPTION_SFX, true);
        } else {
            play_audio(GARBAGE_SFX, true);
            goto main_menu_loop_start;
        }

        mp_sync_ready = false;
        mp_sync_awaiting = true;
        mp_send_msg_packed(mp_msg_ping, 2);
        sleep_ms(1);

        //sync with other console
        while (!mp_sync_ready) {
            mp_send_msg_packed(mp_msg_ping, 2);

            get_inputs();

            if (cur_inputs.b) {
                mp_sync_awaiting = false;
                multiplayer = false;
                goto main_menu_loop_start;
            }
            
            render_title();
            draw_text("singleplayer", 32, 23, true, UNSELECTED_TEXT);
            draw_text("multiplayer", 32, 15, true, Z_PIECE);
            draw_text("options", 32, 7, true, UNSELECTED_TEXT);
            wait_and_push_frame();
        }

        mp_sync_awaiting = false;
        mp_sync_ready = false;
        multiplayer = true;

        cur_screen = game_screen;
        play_audio(SILENCE_SONG, false);
    }

    if (cur_sel == 2) {
        play_audio(SELECT_OPTION_SFX, true);
        cur_screen = options_screen;
    }
}

int main() {
    stdio_init_all();
    display_init();
    init_inputs();
    init_audio();
    mp_uart_init();

    multicore_launch_core1(display_loop);
    get_target_framerate();
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

        fade(250, 0);
    }

    return 0;
}