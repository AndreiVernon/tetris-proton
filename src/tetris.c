#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "tetris.h"
#include "graphics.h"
#include "input.h"

#define GENERATION_DELAY 0.1  //in s
#define GRAVITY_DELAY 0.4 //secs to fall one row
#define GRAVITY_SOFT_MULT 10 //how much soft drop multiplies gravity by

uint8_t matrix[M_HEIGHT][M_WIDTH] = {0};
uint32_t score = 0;
volatile bool game_over = false;
Piece active_piece = {0};       //currently active piece
Piece ghost_piece = {0};        //ghost piece / shadow of active piece
int held_piece_shape = INACTIVE;      //shape of held piece
bool hold_avail = true;         //can hold piece
int rand_bag[14] = {0};         //bag of upcoming pieces
int rand_bag_loc = 0;           //index of bag
GamePhase cur_phase = GENERATION;

int frame_timer;

int generation_timer_flag = -1;     //-1 = not armed, 0 = armed, 1 = fired
repeating_timer_t generation_timer = {0};

int gravity_timer_flag = -1;     //-1 = not armed, 0 = normal, 1 = soft drop
int gravity_count = 0;
repeating_timer_t gravity_timer = {0};
bool soft_drop_active = false;

const int piece_mask_sizes[7] = {5, 2, 3, 3, 3, 3, 3};
//from bottom left to top right
const uint8_t piece_masks[7][25] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // I (0)
    {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // O (1)
    {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // S (2)
    {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // Z (3)
    {0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // T (4)
    {0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // L (5)
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}   // J (6)
};  //      ||       ||       || (3x3)

static const int8_t rotate_offset_data[4][5][2] = {
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},   //0
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},  //R
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},   //2
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}//L
};

//ccw at even indices, cw at odd indices
static const int8_t rotate_offset_data_i_base[8][2] = {
    {0, -1},    //0->L
    {1, 0},     //0->R
    {-1, 0},    //R->0
    {0, -1},    //R->2
    {0, 1},     //2->R
    {-1, 0},    //2->L
    {1, 0},     //L->2
    {0, 1}      //L->0
};

static const int8_t rotate_offset_data_i_arika[8][5][2] = {
    {{0,0}, {2,0}, {-1,0}, {-1,2}, {2,-1}},   //0->L
    {{0,0}, {-2,0}, {1,0}, {1,2}, {-2,-1}},   //0->R
    {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2}},   //R->0
    {{0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1}},   //R->2
    {{0,0}, {-2,0}, {1,0}, {-2,1}, {1,-1}},   //2->R
    {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-1}},   //2->L
    {{0,0}, {1,0}, {-2,0}, {1,2}, {-2,-1}},   //L->2
    {{0,0}, {-2,0}, {1,0}, {-2,1}, {1,-2}}    //L->0
};

void gen_rand_bag(bool second_half);

void reset_game()
{
    //clear the playfield
    memset(matrix, EMPTY, sizeof(matrix));

    score = 0;
    game_over = false;

    //clear active and ghost pieces
    memset(&active_piece, 0, sizeof(active_piece));
    memset(&ghost_piece, 0, sizeof(ghost_piece));

    active_piece.shape = INACTIVE;
    held_piece_shape = INACTIVE;
    hold_avail = true;

    //set up random bag
    rand_bag_loc = 0;
    gen_rand_bag(false);
    gen_rand_bag(true);

    cur_phase = GENERATION;

    generation_timer_flag = -1;
    memset(&generation_timer, 0, sizeof(generation_timer));

    gravity_timer_flag = -1;
    gravity_count = 0;
    memset(&gravity_timer, 0, sizeof(gravity_timer));
    soft_drop_active = false;
}

//check if current piece is colliding with blocks on playfield
//returns true if colliding or out of bounds
bool is_colliding(bool ghost) {
    Piece cur_piece = ghost ? ghost_piece : active_piece;

    for (int y = 0; y < cur_piece.size; y++) {
        for (int x = 0; x < cur_piece.size; x++) {
            //check every active block in piece mask
            if (cur_piece.mask[y * cur_piece.size + x]) {
                //check for oob
                if (cur_piece.x + x < 0 || cur_piece.x + x >= M_WIDTH || cur_piece.y + y < 0 || active_piece.y + y >= M_HEIGHT)
                    return true;

                //check for collision
                if (matrix[cur_piece.y + y][cur_piece.x + x] != EMPTY)
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

        if (oob) game_over = true;
    }

    hold_avail = true;
    active_piece.shape = INACTIVE;
}

//generates random bag of tetriminos
void gen_rand_bag(bool second_half) {
    int *arr = &rand_bag[second_half ? 7 : 0];

    //fill array with numbers 0-6
    for (int i = 0; i < 7; i++)
        arr[i] = i;

    //Fisher-Yates shuffle (uniform sampling)
    for (int i = 6; i > 0; i--) {
        uint32_t j;
        int limit = UINT32_MAX - (UINT32_MAX % (i + 1));

        do {
            j = get_rand_32();
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
    game_over = is_colliding(false);
}

//move piece left or right
//dir: 0 = left, 1 = right
void move(bool dir) {
    int x_old = active_piece.x;

    if (dir) active_piece.x++;
    else active_piece.x--;

    if (is_colliding(false)) active_piece.x = x_old;
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
            
            if (!is_colliding(false)) {
                success = true;
                break;
            }
        }
    } else {
        for (int i = 0; i < 5; i++) {
            active_piece.x = x_old + rotate_offset_data[active_piece.rotation][i][0] - rotate_offset_data[rotation_target][i][0];
            active_piece.y = y_old + rotate_offset_data[active_piece.rotation][i][1] - rotate_offset_data[rotation_target][i][1];
            
            if (!is_colliding(false)) {
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

bool is_touching_surface() {
    active_piece.y--;
    bool temp = is_colliding(false);
    active_piece.y++;

    return temp;
}

//drop piece instantly
void hard_drop(bool ghost) {
    Piece* piece_sel = ghost ? &ghost_piece : &active_piece;

    if (piece_sel->shape == INACTIVE) return;

    int temp = piece_sel->y;
    while (!is_colliding(ghost)) piece_sel->y--;

    if (temp != piece_sel->y) piece_sel->y++;
}

//update ghost piece
void update_ghost() {
    ghost_piece = active_piece;
    if (ghost_piece.shape != INACTIVE) ghost_piece.shape = GHOST;
    hard_drop(true);
}

//hold / swap held piece
void hold_piece() {
    if (!hold_avail) return;
    else hold_avail = false;

    int temp = held_piece_shape;
    held_piece_shape = active_piece.shape;

    //if originally held piece is INACTIVE (-1), new piece will come from bag
    active_piece.shape = temp;
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

//drop piece slowly
//returns true if piece ended up touching surface
bool drop_piece(int drop_count) {
    int dropped;
    for (dropped = 0; dropped < drop_count; dropped++) {
        active_piece.y--;

        if (is_colliding(false)) {
            active_piece.y++;
            return true;
        }
    }
    
    return false;
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
    //hold piece
    if (cur_inputs.hold && hold_avail) {
        cancel_repeating_timer(&generation_timer);
        generation_timer_flag = -1;
        hold_piece();
    }

    //arm timer
    if (generation_timer_flag == -1) {
        generation_timer_flag = 0;
        add_repeating_timer_ms(GENERATION_DELAY * 1000, oneshot_cb, &generation_timer_flag, &generation_timer);
    }

    //timer has fired, now disarm
    if (generation_timer_flag == 1) {
        generation_timer_flag = -1;
        spawn_piece(active_piece.shape);

        //guide says "immediately drops one row" if it has space
        drop_piece(1);

        cur_phase = FALLING;
    }
}

//callback function specifically for gravity timer calls
bool gravity_cb(repeating_timer_t *rt) {
    int *flag = (int *) rt->user_data;
    *flag += 1; //fired

    //repeating
    return true;
}

void cancel_gravity() {
    cancel_repeating_timer(&gravity_timer);
    gravity_timer_flag = -1;
}

void update_falling() {

    //soft drop no longer pressed
    if (gravity_timer_flag == 1 && !cur_inputs.soft_drop) {
        cancel_gravity();
        gravity_timer_flag = -1;
    }

    if (gravity_timer_flag == -1) {
        //arm timer
        gravity_timer_flag = 0;
        add_repeating_timer_ms(GRAVITY_DELAY * 1000, gravity_cb, &gravity_count, &gravity_timer);
    }

    //hold piece
    if (cur_inputs.hold && hold_avail) {
        cancel_gravity();
        hold_piece();
        cur_phase = GENERATION;
        return;
    }
    
    //hard drop
    if (cur_inputs.hard_drop) {
        cancel_gravity();
        hard_drop(false);
        cur_phase = CLEAR;
        return;
    }

    //soft drop
    if (cur_inputs.soft_drop && gravity_timer_flag != 1) {
        cancel_gravity();
        add_repeating_timer_ms(GRAVITY_DELAY * 1000 / GRAVITY_SOFT_MULT, gravity_cb, &gravity_count, &gravity_timer);
        gravity_timer_flag = 1;
    }

    //movement
    if (cur_inputs.left || cur_inputs.right) {
        move(cur_inputs.right);
    }

    //rotation
    if (cur_inputs.rot_left || cur_inputs.rot_right) {
        rotate(cur_inputs.rot_right);
    }

    //perform inputs before applying gravity so game feels more responsive
    if (gravity_count >= 1) {
        int drop_count = gravity_count;

        //timer has fired 1 or more times, reset
        gravity_count -= drop_count;

        //try to drop piece, if it touches ground in process, switch to lock phase
        if (drop_piece(drop_count)) {
            cancel_gravity();
            cur_phase = LOCK;
        }

        //no point in checking if touching surface again
        return;
    }

    //check if piece touching surface
    active_piece.y += 1;
    if (is_colliding(false)) {
        cancel_gravity();
        cur_phase = LOCK;
    }
    active_piece.y -= 1;
}

void update_lock() {
    cur_phase = CLEAR;
}

void update_clear() {
    lock_piece();
    cur_phase = GENERATION;
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

    update_ghost();

};

int game_loop() {
    reset_game();

    //game start stuff

    while (!game_over) {
        get_inputs();

        update_game();

        matrix[21][0] = hold_avail ? GARBAGE : EMPTY;

        render_frame();
        while (!frame_ready) tight_loop_contents();
        frame_ready = false;
    }

    //game over stuff

    while (true) render_frame();

    return 0;
}