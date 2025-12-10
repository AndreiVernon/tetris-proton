#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "graphics.h"
#include "tetris.h"
#include "display.h"
#include "assets.h"

#define MATRIX_OFFSET_X 11  //how many px from left pieces are drawn
#define MATRIX_OFFSET_Y 18  //how many px from bottom pieces are drawn

#define FRAME_WIDTH PANEL_WIDTH
#define FRAME_HEIGHT PANEL_HEIGHT
#define FONT_HEIGHT 5
#define LETTER_SPACING 1
#define SPACE_WIDTH 1

volatile bool frame_ready = false;   //frame ready to display
repeating_timer_t frame_timer;
int TARGET_FRAMERATE;

void get_target_framerate() {
    uint32_t start_time = to_us_since_boot(get_absolute_time());
    fbf_swap_request = true;
    while (fbf_swap_request) tight_loop_contents();
    TARGET_FRAMERATE = 1000000 / (to_us_since_boot(get_absolute_time()) - start_time);
}

bool frametime_cb(repeating_timer_t *rt) {
    frame_ready = true;

    //repeating
    return true;
}

void init_frame_timer() {
    add_repeating_timer_us(1000000 / TARGET_FRAMERATE, frametime_cb, NULL, &frame_timer);
}

void wait_and_push_frame() {
    while (!frame_ready) tight_loop_contents();
    frame_ready = false;
    
    fbf_swap_request = true;
    while (fbf_swap_request) tight_loop_contents();
}

void set_pixel_color(uint8_t *arr, uint8_t shape_id, bool dim) {
    switch (shape_id) {
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
            memcpy(arr, ((uint8_t[3])GREY), 3);
            break;
        case GHOST:
            memcpy(arr, ((uint8_t[3])DARK_GREY), 3);
            break;
        case WALL:
            memcpy(arr, ((uint8_t[3])FULL_WHITE), 3);
            break;
        case OOB:
            memcpy(arr, ((uint8_t[3])DARK_RED), 3);
            break;
        case SELECTED_TEXT:
            memcpy(arr, ((uint8_t[3])WARM_WHITE), 3);
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

//coords are for bottom left corner of block
void set_block_color(int frame_y, int frame_x, uint8_t shape_id, bool dim) {
    set_pixel_color(framebuffer[!fbf_rdy][frame_y][frame_x], shape_id, dim);
    set_pixel_color(framebuffer[!fbf_rdy][frame_y][frame_x + 1], shape_id, dim);
    set_pixel_color(framebuffer[!fbf_rdy][frame_y + 1][frame_x], shape_id, dim);
    set_pixel_color(framebuffer[!fbf_rdy][frame_y + 1][frame_x + 1], shape_id, dim);
}

void render_matrix() {
    for (int y = 0; y < M_HEIGHT; y++) {
        for (int x = 0; x < M_WIDTH; x++) {
            if (matrix[y][x] == EMPTY) continue;

            int frame_y = MATRIX_OFFSET_Y + y * 2;
            int frame_x = MATRIX_OFFSET_X + x * 2;

            if (frame_y >= FRAME_HEIGHT || frame_x >= FRAME_WIDTH) continue;
            // || frame_y < 0 || frame_x < 0

            set_block_color(frame_y, frame_x, matrix[y][x], true);
        }
    }
}

//takes piece and renders from bottom left corner at x_offset, y_offset
//sets color based on cur_piece.shape
void render_piece(Piece cur_piece, int x_offset, int y_offset) {
    //-1 means piece is hidden
    if (cur_piece.shape == INACTIVE) return;

    for (int y = 0; y < cur_piece.size; y++) {
        for (int x = 0; x < cur_piece.size; x++) {
            if (!cur_piece.mask[y * cur_piece.size + x]) continue;

            int frame_y = y_offset + (y + cur_piece.y) * 2;
            int frame_x = x_offset + (x + cur_piece.x) * 2;

            if (frame_y >= FRAME_HEIGHT || frame_x >= FRAME_WIDTH || frame_y < 0 || frame_x < 0) continue;

            set_block_color(frame_y, frame_x, cur_piece.shape, false);
        }
    }
}

//place new tetrimino
void init_piece_render(Piece *new_piece, int new_shape) {

    new_piece->shape = new_shape;
    new_piece->rotation = 0;
    new_piece->x = 0;
    new_piece->y = 0;
    new_piece->size = 0;

    if (new_shape == INACTIVE) return;
    
    if (new_piece->shape == I_PIECE) new_piece->size = 5;
    else if (new_piece->shape == O_PIECE) new_piece->size = 2;
    else new_piece->size = 3;
    
    //get shape mask
    memcpy(new_piece->mask, piece_masks[new_piece->shape], new_piece->size * new_piece->size);
}

void render_hold() {
    if (held_piece_shape == INACTIVE) return;

    Piece held_piece;
    init_piece_render(&held_piece, held_piece_shape);

    int x_offset, y_offset;
    if (held_piece_shape == I_PIECE) {
        x_offset = 41;
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
        if (rand_bag[(rand_bag_loc + i) % 14] == INACTIVE) continue;

        Piece cur_piece;
        init_piece_render(&cur_piece, rand_bag[(rand_bag_loc + i) % 14]);

        
        if (cur_piece.shape == I_PIECE) {
            x_offset = 41;
            y_offset -= 6;
        } else if (cur_piece.shape == O_PIECE) {
            x_offset = 45;
            y_offset -= 4;
        } else {
            x_offset = 44;
            y_offset -= 6;
        }

        render_piece(cur_piece, x_offset, y_offset);

        if (cur_piece.shape == I_PIECE) y_offset += 2;
        else if (cur_piece.shape == O_PIECE) y_offset -= 2;
    }
}

//map char to font index (0-37)
//returns -2 for space, -1 for unknown
int char_to_idx(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
        if (c == ':') return 36;
        if (c == '!') return 37;
        if (c == '+') return 38;
        if (c == ' ') return -2;
        return -1;
};

void draw_text(const char *s, int x, int y, bool center, uint8_t shape_id)
{
    //compute total pixel width of the string
    int total_w = 0;
    int len = 0;
    for (const char *p = s; *p; ++p) {
        int idx = char_to_idx(*p);
        if (idx >= 0) {
            total_w += font_widths[idx];
            ++len;
        } else if (idx == -2) {
            total_w += SPACE_WIDTH;
            ++len;
        } else {
            total_w += 1; //unknown char == 1 blank column
            ++len;
        }
    }

    if (len > 0) total_w += (len - 1) * LETTER_SPACING;

    //determine bottom-left origin for drawing
    int origin_x = x;
    int origin_y = y;

    if (center) origin_x = x - (total_w / 2);

    //draw each char left-to-right
    int cursor_x = origin_x;

    for (const char *p = s; *p; ++p) {
        int idx = char_to_idx(*p);

        //space
        if (idx == -2) {
            cursor_x += SPACE_WIDTH;
            cursor_x += LETTER_SPACING;
            continue;
        }

        //unknown char
        if (idx == -1) {
            cursor_x += 1;
            cursor_x += LETTER_SPACING;
            continue;
        }

        int w = font_widths[idx];

        //draw font
        for (int row = 0; row < FONT_HEIGHT; ++row) {
            int fy = origin_y + row;

            if (fy < 0 || fy >= PANEL_HEIGHT) continue;

            for (int col = 0; col < w; ++col) {
                if (!font_mask[idx][row][col]) continue;

                int fx = cursor_x + col;

                if (fx < 0 || fx >= PANEL_WIDTH) continue;

                set_pixel_color(framebuffer[!fbf_rdy][fy][fx], shape_id, false);
            }
        }

        //advance cursor
        cursor_x += w;
        cursor_x += LETTER_SPACING;
    }
}

void render_garbage_queue() {
    //render extra wall
    int y_offset = 17;
    int x_offset = 7;
    for (int y = y_offset; y < FRAME_HEIGHT; y++) {
        for (int x = x_offset; x < x_offset + 3; x++) {
            if (y == y_offset || x == x_offset)
                set_pixel_color(framebuffer[!fbf_rdy][y][x], WALL, false);
        }
    }

    //render dark red pieces
    int garbage_drawn = 0;
    int garbage_target = -garbage_queue;
    for (int y = y_offset + 1; y < FRAME_WIDTH && garbage_drawn < garbage_target; y += 2) {
        set_block_color(y, x_offset + 1, OOB, false);
        garbage_drawn++;
    }
}

void dim_screen(float dim_factor) {
    if (dim_factor == 1.0f) return;

    for (int y = 0; y < FRAME_HEIGHT; y++) {
        for (int x = 0; x < FRAME_WIDTH; x++) {
            for (int px = 0; px < 3; px++) {
                framebuffer[!fbf_rdy][y][x][px] *= dim_factor;
            }
        }
    }
}

//dir: 0 = fadeout, 1 = fadein
void fade(int duration_ms, bool dir) {
    // int frames = (duration_ms / 1000.0f) * TARGET_FRAMERATE + 0.49f;
    int frames = duration_ms / 25; //approx speed of display bitbanging
    if (frames < 1) frames = 1;

    uint8_t *orig = malloc(sizeof(framebuffer[0]));
    if (!orig) {
        display_clear();
        draw_text("fades broke!", 32, 32, true, Z_PIECE);
        sleep_ms(duration_ms);
        return;
    }
    memcpy(orig, framebuffer[!fbf_rdy], sizeof(framebuffer[0]));

    float dim_factor;
    for (int i = 0; i < frames; i++) {
        memcpy(framebuffer[!fbf_rdy], orig, sizeof(framebuffer[0]));

        if (!dir) dim_factor = (frames - i - 1) / (float)frames;
        else dim_factor = (i + 1) / (float)frames;

        dim_screen(dim_factor);
        wait_and_push_frame();
    }

    char text[32];
    uint32_t start_time = to_us_since_boot(get_absolute_time());
    wait_and_push_frame();
    start_time = to_us_since_boot(get_absolute_time()) - start_time;
    snprintf(text, sizeof(text), "%lu", start_time);

    start_time = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time()) < start_time + 2000) {
        memset(framebuffer[!fbf_rdy], 0, sizeof(framebuffer[0]));
        draw_text(text, 20, 20, true, SELECTED_TEXT);
        wait_and_push_frame();
    }

    free(orig);
}

void render_frame() {
    //render background
    memcpy(framebuffer[!fbf_rdy], background, sizeof(framebuffer[!fbf_rdy]));

    render_matrix();

    render_piece(ghost_piece, MATRIX_OFFSET_X, MATRIX_OFFSET_Y);
    render_piece(active_piece, MATRIX_OFFSET_X, MATRIX_OFFSET_Y);

    // render_clear_line();

    render_hold();
    render_next();

    if (multiplayer) render_garbage_queue();

    char text[32];

    //render score
    snprintf(text, sizeof(text), "%06lu", score);
    draw_text(text, 20, 10, true, UNSELECTED_TEXT);

    if (multiplayer) {
    //render time
        uint32_t cur_time;
        if (!game_paused) cur_time = (to_ms_since_boot(get_absolute_time()) - game_start_time) / 1000;
        else cur_time = (game_paused_time - game_start_time) / 1000;
        snprintf(text, sizeof(text), "%02lu:%02lu", cur_time/60, cur_time%60);
        draw_text(text, 20, 4, true, SELECTED_TEXT);
    } else {
        snprintf(text, sizeof(text), "Level %02lu", level);
        draw_text(text, 20, 4, true, SELECTED_TEXT);
    }

    if (game_paused) dim_screen(0.1);
}

void render_title() {
    memcpy(framebuffer[!fbf_rdy], title_background, sizeof(framebuffer[!fbf_rdy]));
}