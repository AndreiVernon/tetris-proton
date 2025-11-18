#include "pico/stdlib.h"
#include <string.h>
#include "graphics.h"
#include "display.h"
#include "assets.h"
#include "tetris.h"

#define MATRIX_OFFSET_X 11  //how many px from left pieces are drawn
#define MATRIX_OFFSET_Y 45  //how many px from bottom pieces are drawn

int cur_frame = 0;          //current frame in 1 sec loop
int second_start_time;      //where the current 1 sec loop starts

void frametime_handler() {
    // acknowledge irq
    hw_clear_bits(&timer0_hw->intr, 1 << 0);

    frame_ready = true;

    if (cur_frame == 0)
        second_start_time = timer_hw->timerawl;

    timer0_hw->alarm[0] = second_start_time + (cur_frame + 1) * 1000000 / TARGET_FRAMERATE;
    cur_frame = (cur_frame + 1) % TARGET_FRAMERATE;
}

void init_frame_timer() {
    // Enable the interrupt for our alarm
    hw_set_bits(&timer0_hw->inte, 1 << 0);
    // Set irq handler for alarm irq
    irq_set_exclusive_handler(TIMER0_IRQ_0, frametime_handler);
    // Enable the alarm irq
    irq_set_enabled(TIMER0_IRQ_0, true);
    // set timer
    timer0_hw->alarm[0] = timer_hw->timerawl + 1000000 / TARGET_FRAMERATE;
}

void render_menu() {

}

void render_background() {
    memcpy(framebuffer, background, sizeof(framebuffer) * PANEL_WIDTH * PANEL_HEIGHT * 3);
}

void set_block_color(uint8_t *arr, uint8_t px, bool dim) {
    switch (px) {
        case I_PIECE:
            memcpy(arr, ((uint8_t[3])LIGHT_BLUE), 3);
            break;
        case O_PIECE:
            memcpy(arr, ((uint8_t[3])YELLOW), 3);
            break;
        case S_PIECE:
            memcpy(arr, ((uint8_t[3])GREEN), 3);
            break;
        case Z_PIECE:
            memcpy(arr, ((uint8_t[3])RED), 3);
            break;
        case T_PIECE:
            memcpy(arr, ((uint8_t[3])PURPLE), 3);
            break;
        case L_PIECE:
            memcpy(arr, ((uint8_t[3])ORANGE), 3);
            break;
        case J_PIECE:
            memcpy(arr, ((uint8_t[3])DARK_BLUE), 3);
            break;
        case GARBAGE:
            memcpy(arr, ((uint8_t[3])RUST), 3);
            break;
        case GHOST:
            memcpy(arr, ((uint8_t[3])DARK_GREY), 3);
            break;
        default:
            memcpy(arr, ((uint8_t[3])BLACK), 3);
    }

    if (dim) {
        arr[0] *= V;
        arr[1] *= V;
        arr[2] *= V;
    }
}

void render_matrix() {
    for (int y = 0; y < M_HEIGHT; y++) {
        for (int x = 0; x < M_WIDTH; x++) {
            if (matrix[y][x] == EMPTY) continue;

            //draw 4 pixels per block
            set_block_color(framebuffer[MATRIX_OFFSET_Y + y * 2][MATRIX_OFFSET_X + x * 2], matrix[y][x], true);
            set_block_color(framebuffer[MATRIX_OFFSET_Y + y * 2][MATRIX_OFFSET_X + x * 2 + 1], matrix[y][x], true);
            set_block_color(framebuffer[MATRIX_OFFSET_Y + y * 2 + 1][MATRIX_OFFSET_X + x * 2], matrix[y][x], true);
            set_block_color(framebuffer[MATRIX_OFFSET_Y + y * 2 + 1][MATRIX_OFFSET_X + x * 2 + 1], matrix[y][x], true);
        }
    }
}

void render_piece(Piece cur_piece) {
    uint8_t col[3];
    set_block_color(col, cur_piece.shape, false);

    for (int y = 0; y < cur_piece.size; y++) {
        for (int x = 0; x < cur_piece.size; x++) {
            if (!cur_piece.mask[y * cur_piece.size + x]) continue;

            //draw 4 pixels per block
            memcpy(framebuffer[MATRIX_OFFSET_Y + (y + cur_piece.y) * 2][MATRIX_OFFSET_X + (x + cur_piece.x) * 2], col, 3);
            memcpy(framebuffer[MATRIX_OFFSET_Y + (y + cur_piece.y) * 2][MATRIX_OFFSET_X + (x + cur_piece.x) * 2 + 1], col, 3);
            memcpy(framebuffer[MATRIX_OFFSET_Y + (y + cur_piece.y) * 2 + 1][MATRIX_OFFSET_X + (x + cur_piece.x) * 2], col, 3);
            memcpy(framebuffer[MATRIX_OFFSET_Y + (y + cur_piece.y) * 2 + 1][MATRIX_OFFSET_X + (x + cur_piece.x) * 2 + 1], col, 3);
        }
    }
}

void render_tetris() {
    render_background();
    render_matrix();
    render_piece(ghost_piece);
    render_piece(piece);
    // render_clear();
    // render_next();
    // render_hold();
    // render_score();
    // render_time();
}