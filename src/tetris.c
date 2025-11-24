#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "tetris.h"
#include "graphics.h"

#define GENERATION_DELAY 0.2  //in s

uint8_t matrix[M_HEIGHT][M_WIDTH] = {0};
uint32_t score = 0;
volatile bool game_over = false;
Piece active_piece = {0};       //currently active piece
Piece ghost_piece = {0};        //ghost piece / shadow of active piece
int held_piece_shape = -1;      //shape of held piece
bool hold_avail = true;         //can hold piece
int rand_bag[14] = {0};         //bag of upcoming pieces
int rand_bag_loc = 0;           //index of bag
GamePhase cur_phase = GENERATION;

int frame_timer;
int generation_timer_flag = -1;     //-1 = not armed, 0 = armed, 1 = fired
repeating_timer_t generation_timer = {0};

const int piece_mask_sizes[7] = {5, 2, 3, 3, 3, 3, 3};
//from bottom left to top right
const uint8_t piece_masks[7][25] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // I (0)
    {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // O (1)
    {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // S (2)
    {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // Z (3)
    {0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // T (4)
    {0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // L (5)
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}   // J (6)
};  //      ||       ||       || (3x3)

const int8_t rotate_offset_data[4][5][2] = {
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},   //0
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},  //R
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},   //2
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}//L
};

//ccw at even indices, cw at odd indices
const int8_t rotate_offset_data_i_base[8][2] = {
    {0, -1},    //0->L
    {1, 0},     //0->R
    {-1, 0},    //R->0
    {0, -1},    //R->2
    {0, 1},     //2->R
    {-1, 0},    //2->L
    {1, 0},     //L->2
    {0, 1}      //L->0
};

const int8_t rotate_offset_data_i_arika[8][5][2] = {
    {{0,0}, {2,0}, {-1,0}, {-1,2}, {2,-1}},   //0->L
    {{0,0}, {-2,0}, {1,0}, {1,2}, {-2,-1}},   //0->R
    {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2}},   //R->0
    {{0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1}},   //R->2
    {{0,0}, {-2,0}, {1,0}, {-2,1}, {1,-1}},   //2->R
    {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-1}},   //2->L
    {{0,0}, {1,0}, {-2,0}, {1,2}, {-2,-1}},   //L->2
    {{0,0}, {-2,0}, {1,0}, {-2,1}, {1,-2}}    //L->0
};

void reset_game()
{
    //clear the playfield
    memset(matrix, 0, sizeof(matrix));

    score = 0;
    game_over = false;

    //clear active and ghost pieces
    memset(&active_piece, 0, sizeof(active_piece));
    memset(&ghost_piece, 0, sizeof(ghost_piece));

    held_piece_shape = -1;
    hold_avail = true;

    //set up random bag
    rand_bag_loc = 0;
    gen_rand_bag(false);
    gen_rand_bag(true);

    cur_phase = GENERATION;

    generation_timer_flag = -1;
    memset(&generation_timer, 0, sizeof(generation_timer));
}

//check if current piece is colliding with blocks on playfield
//returns true if colliding or out of bounds
bool is_colliding() {
    for (int y = 0; y < active_piece.size; y++) {
        for (int x = 0; x < active_piece.size; x++) {
            //check every active block in piece mask
            if (active_piece.mask[y * active_piece.size + x]) {
                //check for oob
                if (active_piece.x + x < 0 || active_piece.x + x >= M_WIDTH || active_piece.y + y < 0 || active_piece.y + y >= M_HEIGHT)
                    return true;

                //check for collision
                if (matrix[active_piece.y + y][active_piece.x + x] != EMPTY)
                    return true;
            }
        }
    }

    return false;
}

//locks location of the piece and adds to playfield
void lock_piece() {
    for (int y = 0; y < active_piece.size; y++) {
        for (int x = 0; x < active_piece.size; x++) {
            if (active_piece.mask[y * active_piece.size + x]) {
                matrix[active_piece.y + y][active_piece.x + x] = active_piece.shape;
            }
        }
    }

    //game over check
    //only game over if every block in piece is oob
    if (active_piece.y >= 20 - active_piece.size) {
        bool oob = true;

        for (int y = 0; y < active_piece.size; y++) {
            for (int x = 0; x < active_piece.size; x++) {
                if (active_piece.mask[y * active_piece.size + x]) {
                    if (active_piece.y + y < 20) {
                        oob = false;
                        goto nested_break;
                    }
                }
            }
        }
        nested_break:

        hold_avail = true;

        if (oob) game_over = true;
    }
}

//generates random bag of tetriminos
void gen_rand_bag(bool second_half) {
    int *arr = &rand_bag[second_half ? 7 : 0];

    //fill array with numbers 0-6
    for (int i = 0; i < 7; i++)
        arr[i] = i;

    //Fisher-Yates shuffle (uniform sampling)
    for (int i = 6; i > 0; i--) {
        int j;
        int limit = UINT32_MAX - (UINT32_MAX % (i + 1));

        do {
            j = rand();
        } while (j >= limit);

        j %= (i + 1);

        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

//place new tetrimino
void spawn_piece(int new_shape) {
    //default
    if (new_shape == -1) active_piece.shape = rand_bag[rand_bag_loc++];
    //called by hold_piece
    else active_piece.shape = new_shape;

    active_piece.rotation = 0;
    
    //I piece is 5x5
    if (active_piece.shape == I_PIECE) {
        active_piece.x = 2;
        active_piece.y = 18;
        active_piece.size = 5;
    } else if (active_piece.shape == O_PIECE) {
        active_piece.x = 4;
        active_piece.y = 20;
        active_piece.size = 2;
    } else {
        active_piece.x = 3;
        active_piece.y = 19;
        active_piece.size = 3;
    }
    
    //get shape mask
    memcpy(active_piece.mask, piece_masks[active_piece.shape], active_piece.size * active_piece.size);

    //got to end of current bag
    if (rand_bag_loc >= 7) {
        //move second half pieces to first half
        for (int i = 0; i < 7; i++)
            rand_bag[i] = rand_bag[i+7];
        
        rand_bag_loc = 0;
        gen_rand_bag(true);
    }

    //game over check
    game_over = is_colliding();
}

//move piece left or right
//dir: 0 = left, 1 = right
void move(bool dir) {
    int x_old = active_piece.x;

    if (dir) active_piece.x++;
    else active_piece.x--;

    if (is_colliding()) active_piece.x = x_old;
}

//rotate piece
//cw: rotate clockwise
void rotate(bool cw) {
    if (active_piece.shape == O_PIECE) return;

    uint8_t mask_old[25];
    memcpy(mask_old, active_piece.mask, active_piece.size * active_piece.size);
    int size = active_piece.size;

    //perform matrix rotation on piece mask
    if (cw) {
        for (int r = 0; r < size; r++) {
            for (int c = 0; c < size; c++) {
                active_piece.mask[c * size + (size - 1 - r)] = mask_old[r * size + c];
            }
        }
    } else {
        for (int r = 0; r < size; r++) {
            for (int c = 0; c < size; c++) {
                active_piece.mask[(size - 1 - c) * size + r] = mask_old[r * size + c];
            }
        }
    }
    
    int rotation_target = (active_piece.rotation + (cw ? 1 : -1)) % 4;
    int x_old = active_piece.x;
    int y_old = active_piece.y;
    bool success = false;

    //add piece offsets
    if (active_piece.shape == I_PIECE) {
        for (int i = 0; i < 5; i++) {
            active_piece.x = x_old + rotate_offset_data_i_base[active_piece.rotation*2 + cw][0] + rotate_offset_data_i_arika[active_piece.rotation*2 + cw][i][0];
            active_piece.y = y_old + rotate_offset_data_i_base[active_piece.rotation*2 + cw][1] + rotate_offset_data_i_arika[active_piece.rotation*2 + cw][i][1];
            
            if (!is_colliding()) {
                success = true;
                break;
            }
        }
    } else {
        for (int i = 0; i < 5; i++) {
            active_piece.x = x_old + rotate_offset_data[active_piece.rotation][i][0] - rotate_offset_data[rotation_target][i][0];
            active_piece.y = y_old + rotate_offset_data[active_piece.rotation][i][1] - rotate_offset_data[rotation_target][i][1];
            
            if (!is_colliding()) {
                success = true;
                break;
            }
        }
    }

    //if nothing worked, undo
    if (!success) {
        active_piece.x = x_old;
        active_piece.y = y_old;
        memcpy(active_piece.mask, mask_old, active_piece.size * active_piece.size);
        return;
    }

    //if succeeded, set new rotation state
    active_piece.rotation = rotation_target;
}

//drop piece slowly
void soft_drop() {
    active_piece.y--;
    if (is_colliding()) active_piece.y++;
}

//drop piece instantly
void hard_drop(bool ghost) {
    Piece* piece_sel = ghost ? &ghost_piece : &active_piece;

    //go from -2 to height+2 cause thats the highest/lowest a piece can be within mask
    for (int y = -2; y < M_HEIGHT+2; y++) {
        piece_sel->y = y;
        if (!is_colliding()) break;
    }

    if (!ghost) lock_piece();
}

//update ghost piece
void update_ghost() {
    ghost_piece = active_piece;
    ghost_piece.shape = GHOST;
    hard_drop(true);
}

void do_gravity() {

}

//hold / swap held piece
void hold_piece() {
    if (!hold_avail) return;
    else hold_avail = false;

    int temp = held_piece_shape;
    held_piece_shape = active_piece.shape;

    //if originally held piece is -1 (empty), new piece will come from bag
    spawn_piece(temp);
    return;
}

//shifts every line above `row` by amount
void shift_lines(int row, int amount) {
    if (amount == 0) return;

    //direction matters, avoid overwriting line data
    if (amount > 0) {
        for (int y = M_HEIGHT - 1; y >= row; y--) {
            //avoid out of bounds access
            if ((y - amount >= M_HEIGHT) || (y - amount < 0))
                memset(matrix[y], EMPTY, M_WIDTH);
            else
                memcpy(matrix[y], matrix[y - amount], M_WIDTH);
        }
    } else {
        for (int y = row; y < M_HEIGHT; y++) {
            //avoid out of bounds access
            if ((y - amount >= M_HEIGHT) || (y - amount < 0))
                memset(matrix[y], EMPTY, M_WIDTH);
            else
                memcpy(matrix[y], matrix[y - amount], M_WIDTH);
        }
    }

    //game over check
    //only game over if shift up + piece oob,
    //regardless of whether shift caused piece to be oob
    if (amount > 0) {
        for (int y = 20; y < M_HEIGHT; y++) {
            for (int x = 0; x < M_WIDTH; x++) {
                if (x != EMPTY) {
                    game_over = true;
                    goto nested_break;
                }
            }
        }
    }
    nested_break:
    return;

}

//checks for completed lines and removes them
void check_lines() {
    for (int y = 0; y < M_HEIGHT; y++) {
        int cleared_cnt = 0;

        for (int x = 0; x < M_WIDTH; x++) {
            if (matrix[y][x] == EMPTY)
                break;
            else
                cleared_cnt++;
        }

        //line has been cleared
        if (cleared_cnt == M_WIDTH) {
            shift_lines(y, -1);
            //TODO: score up
        }
    }
}

//add garbage in multiplayer moded
void add_garbage() {
    //gonna have to come up with garbage shapes
    //shift_lines(0, size_of_garbage);
}

/* //convert xy coordinates to index of playfield matrix
int coord_to_matrix(int x, int y) {
    return (M_HEIGHT - y - 1) * M_WIDTH + x;
} */

//callback function for add_repeating_timer calls
bool oneshot_cb(repeating_timer_t *rt) {
    int *flag = (int *) rt->user_data;
    *flag = 1; //fired

    //oneshot
    return false;
}

void update_generation() {

    //TODO: handle input

    if (0 && hold_avail) { //hold piece
        cancel_repeating_timer(&generation_timer);
        generation_timer_flag = -1;
        hold_piece();
    }

    if (generation_timer_flag == -1) {
        //arm timer
        generation_timer_flag = 0;
        add_repeating_timer_ms(GENERATION_DELAY * 1000, oneshot_cb, &generation_timer_flag, &generation_timer);
    }

    if (generation_timer_flag == 1) {
        //timer has fired, now disarm
        generation_timer_flag = -1;

        spawn_piece(-1);

        cur_phase = FALLING;
    }
}

void update_falling() {

    //start gravity timer

    //TODO: handle input

    if (0 && hold_avail) { //hold piece
        cur_phase = GENERATION;
        return;
    }
    
    if (0) { //hard drop
        hard_drop(false);
        cur_phase = CLEAR;
        return;
    }

    if (0) { //soft drop

    }

    if (0) { //movement

    }

    if (0) { //rotation

    }

    active_piece.y += 1;
    if (is_colliding()) {
        cur_phase = LOCK;
    }
    active_piece.y -= 1;
}

void update_lock() {
    //cancel gravity timer
}

void update_clear() {

}

void update_game() {

    GamePhase prev_phase;

    do {
        prev_phase = cur_phase;

        switch (cur_phase) {
            case GENERATION:
                update_generation();
                break;
            case FALLING:
                update_falling();
                break;
            case LOCK:
                update_lock();
                break;
            case CLEAR:
                update_clear();
                break;
        }
        
    } while (prev_phase != cur_phase);
    //can continue when phase stays the same

};

int game_loop() {
    reset_game();

    //game start stuff

    while (!game_over) {
        //read_input();

        update_game();

        render_frame();
        while (!frame_ready) tight_loop_contents();
        frame_ready = false;
        //display_frame here (change which framebuffer display.c reads from)
    }

    //game over stuff

    return 0;
}