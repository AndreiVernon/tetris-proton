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
    mp_drain_rx();

    int cur_sel = 0;
    int NUM_OPTS = 3;

    render_main_menu(cur_sel, false);
    fade(1250, 1);

    if (song_choice != SILENCE_SONG) play_audio(TITLE_SONG, false);

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

        if (cur_inputs.a || cur_inputs.start) {
            break;
        }

        render_main_menu(cur_sel, false);
        wait_and_push_frame();
    }

    if (cur_sel == 0) {
        multiplayer = false;
        cur_screen = game_screen;
        play_audio(SELECT_OPTION_SFX, true);
    }

    if (cur_sel == 1) {
        mp_drain_rx();

        //detect other console
        if (mp_handshake_blocking(1000)) {
            play_audio(SELECT_OPTION_SFX, true);
        } else {
            play_audio(PIECE_LOCK_SFX, true);
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
                play_audio(PIECE_LOCK_SFX, true);
                goto main_menu_loop_start;
            }
            
            render_main_menu(cur_sel, true);
            wait_and_push_frame();
        }

        mp_sync_awaiting = false;
        mp_sync_ready = false;
        multiplayer = true;

        cur_screen = game_screen;
    }

    if (cur_sel == 2) {
        play_audio(SELECT_OPTION_SFX, true);
        cur_screen = options_screen;
    }

    play_audio(SILENCE_SONG, false);
}

void options_loop() {
    int cur_sel = 0;
    int NUM_OPTS = 5;

    //0 - song
    //1 - starting level
    //2 - goal (variable vs fixed)
    //3 - mp gravity time
    //4 - back
    
    render_options(cur_sel);
    fade(250, 1);

    play_audio(song_choice, false);

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

        //numerical adjustment
        if (cur_inputs.left || cur_inputs.right) {
            if (cur_sel == 1) {
                if (cur_inputs.left && start_level > 1) {
                    start_level--;
                    play_audio(MOVE_SFX, true);
                } else if (cur_inputs.right && start_level < 15) {
                    start_level++;
                    play_audio(MOVE_SFX, true);
                } else {
                    play_audio(PIECE_LOCK_SFX, true);
                }
            }

            if (cur_sel == 3) {
                if (cur_inputs.left && mp_level_timer > 1) {
                    mp_level_timer--;
                    play_audio(MOVE_SFX, true);
                } else if (cur_inputs.right && mp_level_timer < 41) {
                    mp_level_timer++;
                    play_audio(MOVE_SFX, true);
                } else {
                    play_audio(PIECE_LOCK_SFX, true);
                }
                
            }
        }

        //specific options adjustment
        if (cur_inputs.left_edge || cur_inputs.right_edge) {
            if (cur_sel == 0) {
                if (cur_inputs.left_edge) song_choice = (song_choice - 1 + 4) % 4;
                if (cur_inputs.right_edge) song_choice = (song_choice + 1 + 4) % 4;
                play_audio(song_choice, false);
            }

            if (cur_sel == 2) {
                fixed_level_system = !fixed_level_system;
                play_audio(SWITCH_OPTION_SFX, true);
            }
        }

        //go back
        if ((cur_sel == 4 && (cur_inputs.a || cur_inputs.start)) || cur_inputs.b) {
            play_audio(SELECT_OPTION_SFX, true);
            break;
        }

        render_options(cur_sel);
        wait_and_push_frame();
    }

    play_audio(SILENCE_SONG, false);
    cur_screen = title_screen;
}

void game_over_loop() {
    int cur_sel = 0;
    int NUM_OPTS = 2;

    bool game_was_mp = multiplayer;
    multiplayer = false;

    uint32_t gameover_start_time = to_ms_since_boot(get_absolute_time());;

    //sp
    //0 retry
    //1 quit

    //mp
    //0 rematch
    //1 quit
    
    render_game_over(cur_sel, false, game_was_mp);
    fade(250, 1);

    if (song_choice != SILENCE_SONG) play_audio(ENDING_SONG, false);

    while (1) {
        game_over_loop_start:
        get_inputs();

        if (((to_ms_since_boot(get_absolute_time()) - gameover_start_time) / 1000) > 20) {
            cur_sel = 1;
            break;
        }

        if (cur_inputs.up) {
            cur_sel = (cur_sel - 1 + NUM_OPTS) % NUM_OPTS;
            play_audio(SWITCH_OPTION_SFX, true);
        }

        if (cur_inputs.down) {
            cur_sel = (cur_sel + 1 + NUM_OPTS) % NUM_OPTS;
            play_audio(SWITCH_OPTION_SFX, true);
        }

        if (cur_inputs.a || cur_inputs.start) {
            break;
        }

        render_game_over(cur_sel, false, game_was_mp);
        wait_and_push_frame();
    }

    if (cur_sel == 1) {
        cur_screen = title_screen;
        play_audio(SELECT_OPTION_SFX, true);
    }

    if (!game_was_mp && cur_sel == 0) {
        cur_screen = game_screen;
        play_audio(SELECT_OPTION_SFX, true);
    }

    if (game_was_mp && cur_sel == 0) {
        mp_drain_rx();

        //detect other console
        if (mp_handshake_blocking(1000)) {
            play_audio(SELECT_OPTION_SFX, true);
        } else {
            play_audio(PIECE_LOCK_SFX, true);
            goto game_over_loop_start;
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
                play_audio(PIECE_LOCK_SFX, true);
                goto game_over_loop_start;
            }
            
            render_game_over(cur_sel, true, game_was_mp);
            wait_and_push_frame();
        }

        mp_sync_awaiting = false;
        mp_sync_ready = false;
        multiplayer = true;

        cur_screen = game_screen;
    }

    play_audio(SILENCE_SONG, false);
}

int main() {
    stdio_init_all();

    // display_init();
    // multicore_launch_core1(display_loop);
    pio_init();
    multicore_launch_core1(pio_loop);

    init_inputs();
    init_audio();
    mp_uart_init();

    
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
                game_over_loop();
                break;
            case options_screen:
                options_loop();
                break;
        }

        fade(350, 0);
    }

    return 0;
}