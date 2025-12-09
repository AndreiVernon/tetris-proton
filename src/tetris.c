#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "tetris.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"
#include "multiplayer.h"

#define GENERATION_DELAY 0.1    //in s
#define MP_LEVEL_TIMER 20        //how long it takes to increase level by 1 in multiplayer
#define GRAVITY_INCREASE 0.007
#define GRAVITY_SOFT_MULT 20    //how much soft drop multiplies gravity by

#define LOCK_DOWN_TIMER 0.5     //in s
#define LOCK_RESET_LIMIT 15

uint8_t matrix[M_HEIGHT][M_WIDTH] = {0};

uint32_t score = 0;
uint32_t level = 1;
double gravity = 1; //secs to fall one row
volatile int game_over = 0;

Piece active_piece = {0};       //currently active piece
Piece ghost_piece = {0};        //ghost piece / shadow of active piece

int held_piece_shape = INACTIVE;      //shape of held piece
bool hold_avail = true;         //can hold piece

int rand_bag[14] = {0};         //bag of upcoming pieces
int rand_bag_loc = 0;           //index of bag

GamePhase cur_phase = GENERATION;

int frame_timer;
bool game_paused = false;

uint32_t game_start_time;   //in ms

int generation_timer_flag = -1;     //-1 = not armed, 0 = armed, 1 = fired
repeating_timer_t generation_timer = {0};

int gravity_timer_flag = -1;     //-1 = not armed, 0 = normal, 1 = soft drop
int gravity_count = 0;
repeating_timer_t gravity_timer = {0};
bool soft_drop_active = false;

int lock_timer_flag = -1;     //-1 = not armed, 0 = armed, 1 = fired
repeating_timer_t lock_timer = {0};
int lock_reset_count = 0;     //number of times lock timer has been reset
int lowest_height_reached = M_HEIGHT;

bool mp_test_en = false;
bool multiplayer = false;
volatile int garbage_queue = 0;   //neg is receiving, pos is sending

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

void init_game_blank() {
    memset(matrix, EMPTY, sizeof(matrix));
    memset(rand_bag, INACTIVE, sizeof(rand_bag));
}

void update_gravity() {
    //(0.8 - ((level - 1) * 0.007))^(level - 1)
    gravity = pow((0.8 - ((level - 1) * GRAVITY_INCREASE)), (level - 1));
}

//returns time in s
uint32_t get_game_time() {
    return to_ms_since_boot(get_absolute_time()) - game_start_time;
}

void reset_game() {
    //clear the playfield
    memset(matrix, EMPTY, sizeof(matrix));

    score = 0;
    level = 1;
    game_over = 0;
    game_paused = false;
    
    update_gravity();

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

    lock_timer_flag = -1;
    memset(&lock_timer, 0, sizeof(lock_timer));
    lock_reset_count = 0;
    lowest_height_reached = M_HEIGHT;
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
                if (cur_piece.x + x < 0 || cur_piece.x + x >= M_WIDTH || cur_piece.y + y < 0 || cur_piece.y + y >= M_HEIGHT+5)
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

        if (oob) game_over += 1;
    }

    play_audio(PIECE_LOCK_SFX, true);

    hold_avail = true;
    active_piece.shape = INACTIVE;
}

//generates number between 0..max-1
uint32_t get_rand_32_uniform_scaled(uint32_t max) {
    uint32_t j;
    int limit = UINT32_MAX - (UINT32_MAX % max);

    do {
        j = get_rand_32();
    } while (j >= limit);

    j %= max;

    return j;
}

//generates random bag of tetriminos
void gen_rand_bag(bool second_half) {
    int *arr = &rand_bag[second_half ? 7 : 0];

    //fill array with numbers 0-6
    for (int i = 0; i < 7; i++)
        arr[i] = i;

    //Fisher-Yates shuffle (uniform sampling)
    for (int i = 6; i > 0; i--) {
        int j = get_rand_32_uniform_scaled(i + 1);

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
    game_over += is_colliding(false);

    lowest_height_reached = active_piece.y;
}

//move piece left or right
//dir: 0 = left, 1 = right
bool move(bool dir) {
    int x_old = active_piece.x;

    if (dir) active_piece.x++;
    else active_piece.x--;

    if (is_colliding(false)) {
        active_piece.x = x_old;
        return false;
    }

    play_audio(MOVE_SFX, true);
    return true;
}

//rotate piece
//cw: rotate clockwise
bool rotate(bool cw) {
    if (active_piece.shape == O_PIECE) return false;

    uint8_t mask_old[25];
    int size = active_piece.size;
    memcpy(mask_old, active_piece.mask, sizeof(mask_old));
    memset(active_piece.mask, 0, sizeof(active_piece.mask));

    //perform matrix rotation on piece mask
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {

            uint8_t val = mask_old[y * size + x];
            if (!val) continue;

            //this is flipped from what other sources will say
            //because y=0 is bottom instead of top
            int new_x, new_y;
            if (cw) {
                new_x = y;
                new_y = (size - 1) - x;
            } else {
                new_x = (size - 1) - y;
                new_y = x;
            }

            active_piece.mask[new_y * size + new_x] = val;
        }
    }
    
    int rotation_target = (active_piece.rotation + (cw ? 1 : -1) + 4) % 4;
    int x_old = active_piece.x;
    int y_old = active_piece.y;
    bool success = false;

    //add piece offsets
    if (active_piece.shape == I_PIECE) {
        int table_index = active_piece.rotation*2 + cw;

        for (int i = 0; i < 5; i++) {
            active_piece.x = x_old + rotate_offset_data_i_base[table_index][0] + rotate_offset_data_i_arika[table_index][i][0];
            active_piece.y = y_old + rotate_offset_data_i_base[table_index][1] + rotate_offset_data_i_arika[table_index][i][1];
            
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
        memcpy(active_piece.mask, mask_old, sizeof(active_piece.mask));
        return false;
    }

    //if succeeded, set new rotation state
    active_piece.rotation = rotation_target;

    play_audio(ROTATE_SFX, true);
    return true;
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

    play_audio(SWITCH_OPTION_SFX, true);

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
                if (matrix[y][x] != EMPTY) {
                    game_over += 1;
                    goto nested_break;
                }
            }
        }
    }
    nested_break:
    return;

}

//checks for completed lines and removes them
int check_lines() {
    int cleared = 0;
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
            //next line was pulled down to current y, don't skip it
            y--;
            cleared++;
        }
    }

    if (cleared >= 4) play_audio(CLEAR4_SFX, true);
    else if (cleared > 0) play_audio(CLEAR_SFX, true);

    return cleared;
}

//drop piece slowly
//returns true if piece ended up colliding
bool drop_piece(int drop_count) {
    int dropped;
    bool ret = false;

    for (dropped = 0; dropped < drop_count; dropped++) {
        active_piece.y--;

        if (is_colliding(false)) {
            active_piece.y++;
            ret = true;
            break;
        }
    }

    if (active_piece.y < lowest_height_reached) {
        lock_reset_count = 0;
        lowest_height_reached = active_piece.y;
    }
    
    return ret;
}

//add garbage to matrix
void add_garbage() {
    if (!multiplayer) return;
    if (garbage_queue >= 0) return;

    play_audio(GARBAGE_SFX, true);

    //randomly select gap block
    int gap_x = get_rand_32_uniform_scaled(M_WIDTH);

    while (garbage_queue < 0) {
        shift_lines(0, 1);

        //create garbage blocks
        for (int x = 0; x < M_WIDTH; x++) {
            if (x != gap_x) matrix[0][x] = GARBAGE;
        }

        //if garbage covers active piece, push it out of the floor
        if (is_colliding(false)) active_piece.y++;

        garbage_queue++;
    }
}

void send_garbage(int amount) {
    if (!multiplayer) return;

    garbage_queue += amount;

    //if already receiving more than we are sending out
    if (garbage_queue <= 0) return;

    mp_send_msg_packed(mp_msg_send_lines, garbage_queue);
    garbage_queue = 0;
}

//callback function for add_repeating_timer calls
bool oneshot_cb(repeating_timer_t *rt) {
    int *flag = (int *) rt->user_data;
    *flag = 1; //fired

    //oneshot
    return false;
}

void cancel_generation_timer() {
    cancel_repeating_timer(&generation_timer);
    generation_timer_flag = -1;
}

void update_generation() {
    //hold piece
    if (cur_inputs.hold && hold_avail) {
        cancel_generation_timer();
        hold_piece();
    }

    //arm timer
    if (generation_timer_flag == -1) {
        generation_timer_flag = 0;
        add_repeating_timer_ms(GENERATION_DELAY * 1000, oneshot_cb, &generation_timer_flag, &generation_timer);
    }

    //timer has fired, now disarm
    if (generation_timer_flag == 1) {
        cancel_generation_timer();

        update_gravity();

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

    if (gravity_timer_flag == 1) play_audio(SOFT_DROP_SFX, true);

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
    }

    if (gravity_timer_flag == -1) {
        //arm timer
        gravity_timer_flag = 0;
        add_repeating_timer_ms(gravity * 1000, gravity_cb, &gravity_count, &gravity_timer);
    }

    //hold piece
    if (cur_inputs.hold && hold_avail) {
        cancel_gravity();
        hold_piece();
        cur_inputs.hold = false; //consume
        cur_phase = GENERATION;
        return;
    }
    
    //hard drop
    if (cur_inputs.hard_drop) {
        cancel_gravity();
        hard_drop(false);
        cur_inputs.hard_drop = false; //consume
        cur_phase = CLEAR;
        return;
    }

    //soft drop
    if (cur_inputs.soft_drop && gravity_timer_flag != 1) {
        cancel_gravity();
        add_repeating_timer_ms(gravity * 1000 / GRAVITY_SOFT_MULT, gravity_cb, &gravity_count, &gravity_timer);
        gravity_timer_flag = 1;
    }

    //movement
    if (cur_inputs.left || cur_inputs.right) {
        move(cur_inputs.right);

        //consume
        cur_inputs.left = false;
        cur_inputs.right = false;
    }

    //rotation
    if (cur_inputs.rot_left || cur_inputs.rot_right) {
        //cw = true for rightwards rot
        rotate(cur_inputs.rot_right);

        //consume
        cur_inputs.rot_left = false;
        cur_inputs.rot_right = false;
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
            play_audio(TOUCH_SURFACE_SFX, true);
            return;
        }
    }

    //check if touching surface
    if (is_touching_surface()) {
        cancel_gravity();
        cur_phase = LOCK;
        play_audio(TOUCH_SURFACE_SFX, true);
    }
}

void cancel_lock_timer() {
    cancel_repeating_timer(&lock_timer);
    lock_timer_flag = -1;
}

void reset_lock_timer() {
    cancel_repeating_timer(&lock_timer);
    add_repeating_timer_ms(LOCK_DOWN_TIMER * 1000, oneshot_cb, &lock_timer_flag, &lock_timer);
    lock_timer_flag = 0;
    lock_reset_count++;
}

void update_lock() {
    //arm timer
    if (lock_timer_flag == -1) {
        add_repeating_timer_ms(LOCK_DOWN_TIMER * 1000, oneshot_cb, &lock_timer_flag, &lock_timer);
        lock_timer_flag = 0;
    }

    //timer has fired
    if (lock_timer_flag == 1) {
        cancel_lock_timer();
        lock_reset_count = 0;
        cur_phase = CLEAR;
        return;
    }

    //hold piece
    if (cur_inputs.hold && hold_avail) {
        cancel_lock_timer();
        lock_reset_count = 0;
        hold_piece();
        cur_inputs.hold = false; //consume
        cur_phase = GENERATION;
        return;
    }

    //hard drop
    if (cur_inputs.hard_drop) {
        cancel_lock_timer();
        lock_reset_count = 0;
        hard_drop(false);
        cur_inputs.hard_drop = false; //consume
        cur_phase = CLEAR;
        return;
    }

    //movement or rotation
    if (lock_reset_count < LOCK_RESET_LIMIT && (cur_inputs.left || cur_inputs.right || cur_inputs.rot_left || cur_inputs.rot_right)) {
        bool moved = false;

        if (cur_inputs.left || cur_inputs.right) {
            moved = moved || move(cur_inputs.right);
        }

        if (cur_inputs.rot_left || cur_inputs.rot_right) {
            moved = moved || rotate(cur_inputs.rot_right);
        }

        //consume movement input so we arent trapped in inf loop
        cur_inputs.left = false;
        cur_inputs.right = false;
        cur_inputs.rot_left = false;
        cur_inputs.rot_right = false;

        //if move successful and space to fall, go to falling phase
        if (moved && !is_touching_surface()) {
            cancel_lock_timer();
            cur_phase = FALLING;
            return;
        }

        //if move successful and touching surface, reset lock
        else if (moved) {
            reset_lock_timer();
        }
    }
}

bool perfect_clear_check() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < M_WIDTH; x++) {
            if (matrix[y][x] != EMPTY) return false;
        }
    }
    return true;
}

void update_clear() {
    lock_piece();

    int cleared = check_lines();

    //perfect clear
    if (perfect_clear_check()) {
        if (multiplayer) add_garbage(10);
        //TODO also add garbage of last line
    }
    switch (cleared) {
        case 0:
            if (multiplayer) add_garbage();
            break;
        case 1:
            // if (multiplayer) send_garbage(1);
            break;
        case 2:
            if (multiplayer) send_garbage(1);
            break;
        case 3:
            if (multiplayer) send_garbage(2);
            break;
        case 4:
            if (multiplayer) send_garbage(4);
            break;
    }

    //TODO update score and then level

    if (multiplayer) level = (get_game_time() / 1000) / MP_LEVEL_TIMER + 1;

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

void pause_game() {
    if (!game_paused) {
        game_paused = true;

        if (multiplayer && mp_pause_received == 0) mp_send_msg_packed(mp_msg_pause, 0);
        else if (multiplayer && mp_pause_received == 1) mp_pause_received = 0;

        play_audio(PAUSE_SFX, true);
        song_paused = true;

        cancel_generation_timer();
        cancel_gravity();
        cancel_lock_timer();

    } else {
        if (mp_pause_received == -1 || cur_inputs.pause) {
            game_paused = false;

            if (multiplayer && mp_pause_received == 0) mp_send_msg_packed(mp_msg_pause, 1);
            else if (multiplayer && mp_pause_received == -1) mp_pause_received = 0;

            song_paused = false;

            return;
        }
    }
}

void mp_test() {
    int i = 0;

    while (1) {
        received_ping = false;
        get_inputs();

        if (cur_inputs.rot_right) {
            int col;

            if (mp_handshake_blocking(250)) col = S_PIECE;
            else col = Z_PIECE;

            matrix[i / M_WIDTH][i % M_WIDTH] = col;
            i++;
        }

        while (!frame_ready) tight_loop_contents();
        frame_ready = false;

        if (received_ping) {
            matrix[i / M_WIDTH][i % M_WIDTH] = O_PIECE;
            i++;
        }

        render_frame();
    }
}

int game_loop() {
    reset_game();

    render_frame();
    play_audio(GAME_START_SFX, true);
    sleep_ms(3200);

    play_audio(THEMEA_SONG, false);

    if (multiplayer && mp_test_en) mp_test();

    game_start_time = to_ms_since_boot(get_absolute_time());

    while (game_over == 0) {
        get_inputs();

        if (game_paused || cur_inputs.pause || mp_pause_received != 0) pause_game();

        if (!game_paused) update_game();

        render_frame();
        while (!frame_ready) tight_loop_contents();
        frame_ready = false;
    }

    //game over
    if (game_over == 1) {
        mp_send_msg(mp_msg_game_over);
        play_audio(GAME_OVER_SFX, true);
        memset(matrix, Z_PIECE, sizeof(matrix));
    }

    if (game_over == -1) {
        memset(matrix, S_PIECE, sizeof(matrix));
    }

    active_piece.shape = INACTIVE;
    ghost_piece.shape = INACTIVE;

    play_audio(SILENCE_SONG, false);

    while (true) render_frame();

    return 0;
}