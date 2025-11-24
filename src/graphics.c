#include "pico/stdlib.h"
#include <string.h>
#include "graphics.h"
#include "tetris.h"
#include "display.h"
#include "assets.h"

#define MATRIX_OFFSET_X 11  //how many px from left pieces are drawn
#define MATRIX_OFFSET_Y 18  //how many px from bottom pieces are drawn

volatile bool frame_ready = false;   //frame ready to display

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
    memcpy(framebuffer, background, sizeof(framebuffer));
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

            int frame_y = MATRIX_OFFSET_Y + y * 2;
            int frame_x = MATRIX_OFFSET_X + x * 2;

            if (frame_y >= PANEL_HEIGHT || frame_x >= PANEL_WIDTH) continue;
            // || frame_y < 0 || frame_x < 0

            //draw 4 pixels per block
            set_block_color(framebuffer[frame_y][frame_x], matrix[y][x], true);
            set_block_color(framebuffer[frame_y][frame_x + 1], matrix[y][x], true);
            set_block_color(framebuffer[frame_y + 1][frame_x], matrix[y][x], true);
            set_block_color(framebuffer[frame_y + 1][frame_x + 1], matrix[y][x], true);
        }
    }
}

//takes piece and renders from bottom left corner at x_offset, y_offset
//sets color based on cur_piece.shape
void render_piece(Piece cur_piece, int x_offset, int y_offset) {
    //-1 means piece is hidden
    if (cur_piece.shape == -1) return;

    uint8_t col[3];
    set_block_color(col, cur_piece.shape, false);

    for (int y = 0; y < cur_piece.size; y++) {
        for (int x = 0; x < cur_piece.size; x++) {
            if (!cur_piece.mask[y * cur_piece.size + x]) continue;

            int frame_y = y_offset + (y + cur_piece.y) * 2;
            int frame_x = x_offset + (x + cur_piece.x) * 2;

            if (frame_y >= PANEL_HEIGHT || frame_x >= PANEL_WIDTH || frame_y < 0 || frame_x < 0) continue;

            //draw 4 pixels per block
            memcpy(framebuffer[frame_y][frame_x], col, 3);
            memcpy(framebuffer[frame_y][frame_x + 1], col, 3);
            memcpy(framebuffer[frame_y + 1][frame_x], col, 3);
            memcpy(framebuffer[frame_y + 1][frame_x + 1], col, 3);
        }
    }
}

//place new tetrimino
void init_piece_render(Piece *new_piece, int new_shape) {

    new_piece->shape = new_shape;
    new_piece->rotation = 0;
    new_piece->x = 0;
    new_piece->y = 0;
    
    if (new_piece->shape == I_PIECE) new_piece->size = 5;
    else if (new_piece->shape == O_PIECE) new_piece->size = 2;
    else new_piece->size = 3;
    
    //get shape mask
    memcpy(new_piece->mask, piece_masks[new_piece->shape], new_piece->size * new_piece->size);
}

void render_hold() {
    if (held_piece_shape == -1) return;

    Piece held_piece;
    init_piece_render(&held_piece, held_piece_shape);

    int x_offset, y_offset;
    if (held_piece_shape == I_PIECE) {
        x_offset = 43;
        y_offset = 49;
    } else if (held_piece_shape == O_PIECE) {
        x_offset = 45;
        y_offset = 52;
    } else {
        x_offset = 44;
        y_offset = 50;
    }

    render_piece(held_piece, x_offset, y_offset);
}

void render_next() {
    int x_offset, y_offset;
    y_offset = 43;

    for (int i = 0; i < 7; i++) {
        Piece cur_piece;
        init_piece_render(&cur_piece, rand_bag[(rand_bag_loc + i) % 14]);

        
        if (cur_piece.shape == I_PIECE) {
            x_offset = 43;
            y_offset -= 6;
        } else if (cur_piece.shape == O_PIECE) {
            x_offset = 45;
            y_offset -= 4;
        } else {
            x_offset = 44;
            y_offset -= 6;
        }

        render_piece(cur_piece, x_offset, y_offset);

        if (cur_piece.shape == O_PIECE) y_offset -= 2;
    }
}

void render_frame() {
    render_background();
    render_matrix();
    render_piece(ghost_piece, MATRIX_OFFSET_X, MATRIX_OFFSET_Y);
    render_piece(active_piece, MATRIX_OFFSET_X, MATRIX_OFFSET_Y);
    // render_clear_line();
    render_hold();
    render_next();
    // render_score();
    // render_time();
}