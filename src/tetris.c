#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "main.h"
#include "tetris.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"
#include "multiplayer.h"

#define GENERATION_DELAY 0.1    //in s
#define MP_LEVEL_TIMER_DEF 20        //how long it takes to increase level by 1 in multiplayer
#define GRAVITY_INCREASE 0.007
#define GRAVITY_SOFT_MULT 20    //how much soft drop multiplies gravity by

#define LINES_PER_LEVEL 5
#define FIXED_LINES_PER_LEVEL 8

#define LOCK_DOWN_TIMER 0.5     //in s
#define LOCK_RESET_LIMIT 15

uint8_t matrix[M_HEIGHT][M_WIDTH] = {0};

uint32_t score = 0;
uint32_t level = 1;
int total_lines_cleared = 0;
int total_lines_sent = 0;
int total_lines_rcvd = 0;
double gravity = 1; //secs to fall one row
volatile int game_over = 0; //1 = lose, -1 = win, 2 = quit

Piece active_piece = {0};       //currently active piece
Piece ghost_piece = {0};        //ghost piece / shadow of active piece

int held_piece_shape = INACTIVE;      //shape of held piece
bool hold_avail = true;         //can hold piece

int rand_bag[14] = {0};         //bag of upcoming pieces
int rand_bag_loc = 0;           //index of bag

GamePhase cur_phase = GENERATION;

int frame_timer;
int game_paused = 0; //1 = pause, 2 = mp connection lost, 3 = mp connection just lost

uint32_t game_start_time;   //in ms
uint32_t game_paused_time;  //in ms
int final_time; //in s

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

int cur_sel = 0; //for pause menu

int mp_level_timer = MP_LEVEL_TIMER_DEF;
int mp_level_timer_effective = MP_LEVEL_TIMER_DEF;
int start_level = 1;
int start_level_effective = 1;
bool fixed_level_system = true;
volatile bool in_game = false;

//flag set when the active piece was rotated at least once since spawn
bool active_piece_was_rotated = false;
//back-to-back active flag
bool back_to_back_active = false;
//combo counter: -1 means no active combo streak
int combo_counter = -1;
bool active_piece_last_action_was_rotate = false;
int last_rotation_dx = 0; //last rotation wallkick delta x (post-rotation x - pre-rotation x)
int last_rotation_dy = 0; //last rotation wallkick delta y

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
    level = start_level;
    total_lines_cleared = 0;
    update_gravity();

    game_over = 0;
    game_paused = 0;
    game_start_time = to_ms_since_boot(get_absolute_time());

    garbage_queue = 0;
    song_paused = false;

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

    mp_level_timer_effective = mp_level_timer;
    start_level_effective = start_level;

    total_lines_sent = 0;
    total_lines_rcvd = 0;

    final_time = 0;

    back_to_back_active = false;
    combo_counter = -1;
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
    active_piece_was_rotated = false;
    active_piece_last_action_was_rotate = false;
    last_rotation_dx = last_rotation_dy = 0;

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
        for (int i = 0; i < 7; i++) {
            rand_bag[i] = rand_bag[i+7];
        }

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

    last_rotation_dx = active_piece.x - x_old; //record kick delta x
    last_rotation_dy = active_piece.y - y_old; //record kick delta y
    active_piece_last_action_was_rotate = true;
    active_piece_was_rotated = true;

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
    
    if (!ghost) {
        int score_add = 2 * (temp - piece_sel->y);
        if (score_add > 0) score += score_add;
    }
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

    active_piece_was_rotated = false;
    active_piece_last_action_was_rotate = false;
    last_rotation_dx = last_rotation_dy = 0;
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
        total_lines_rcvd++;

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
    total_lines_sent += garbage_queue;
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

    if (gravity_timer_flag == 1) {
        play_audio(SOFT_DROP_SFX, true);
        score++;
    }

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

void calculate_level() {
    int offset;

    if (multiplayer) {
        if (mp_level_timer_effective <= 0 || mp_level_timer_effective >= 41) return;

        if (start_level_effective <= 1) offset = 0;
        else offset = start_level_effective - 1;

        level = get_game_time() / 1000 / mp_level_timer_effective + 1 + offset;

    } else {
        if (fixed_level_system) {
            //(start_level-1)*10
            if (start_level_effective <= 1) offset = 0;
            else offset = (start_level_effective - 1) * FIXED_LINES_PER_LEVEL;

            level = (total_lines_cleared + offset) / FIXED_LINES_PER_LEVEL + 1;
        } else {
            //5 + 10 + ... + 5*(start_level-1)
            if (start_level_effective <= 1) offset = 0;
            else offset = ((start_level_effective - 1) * (5 + 5 * (start_level_effective - 1))) / 2;

            double a = LINES_PER_LEVEL;
            double L = (double)(total_lines_cleared + offset);
            double n_real = (sqrt(a*a + 2.0*a*4.0*L) - a) / (2.0*a);
            level = (int)n_real + 1;
        }
    }

    if (level > 15) level = 15;
    if (level < 0) level = 1;
}

//returns 0 = not a T-Spin, 1 = mini T-Spin, 2 = full T-Spin
int detect_tspin_from_placement(int px, int py) {
    //px,py are bottom-left of the piece in matrix coordinates

    //rule: last maneuver must be a rotation
    if (!active_piece_last_action_was_rotate) return 0;

    int cx = px + 1; //center x
    int cy = py + 1; //center y

    //count diagonally adjacent (corners) occupied => out-of-bounds counts as occupied
    int corners = 0;
    int dx_diag[4] = {-1, 1, -1, 1}; //NW,NE,SW,SE relative offsets for x
    int dy_diag[4] = {1, 1, -1, -1};
    for (int i = 0; i < 4; i++) {
        int x = cx + dx_diag[i];
        int y = cy + dy_diag[i];
        if (x < 0 || x >= M_WIDTH || y < 0 || y >= M_HEIGHT) {
            corners++;
        } else if (matrix[y][x] != EMPTY) {
            corners++;
        }
    }

    //need at least 3 occupied corners to qualify
    if (corners < 3) return 0;

    //determine the T's stem orientation by finding which orthogonal cell around center is EMPTY
    //orth order: up(0), right(1), down(2), left(3)
    int ox[4] = {0, 1, 0, -1};
    int oy[4] = {1, 0, -1, 0};
    int occupied_orth[4] = {0,0,0,0};
    int missing_index = -1;
    for (int i = 0; i < 4; i++) {
        int x = cx + ox[i];
        int y = cy + oy[i];
        if (x < 0 || x >= M_WIDTH || y < 0 || y >= M_HEIGHT) {
            //out of bounds treated as occupied
            occupied_orth[i] = 1;
        } else {
            occupied_orth[i] = (matrix[y][x] != EMPTY) ? 1 : 0;
        }
        if (!occupied_orth[i] && missing_index == -1) missing_index = i;
    }

    //if we couldn't find a single missing orthogonal (external blocks may have filled it),
    //we still can proceed: choose missing_index as the first orthogonal that is 0, otherwise -1
    //if missing_index stays -1, fall back to treating stem as opposite of the least-occupied? but simpler: choose -1->no missing -> can't derive front/back reliably.
    if (missing_index == -1) {
        //we must still determine a stem; however in most valid T placements there will be exactly one missing orthogonal.
        //conservative choice: treat as proper T-Spin (since corners >=3 and last action was rotation),
        //but to follow rules strictly, prefer to evaluate corners pattern below without relying on missing_index.
    }

    //stem direction is opposite of missing orthogonal (if missing known)
    int stem_dir = -1; //0=up,1=right,2=down,3=left
    if (missing_index != -1) stem_dir = (missing_index + 2) % 4;

    //compute front/back corner positions based on stem_dir:
    //if stem up(0): front corners = NW,NE ; back = SW,SE
    //if stem right(1): front = NE,SE ; back = NW,SW
    //if stem down(2): front = SW,SE ; back = NW,NE
    //if stem left(3): front = NW,SW ; back = NE,SE
    int front1x, front1y, front2x, front2y;
    int back1x, back1y, back2x, back2y;

    if (stem_dir == 0) { //up
        front1x = cx-1; front1y = cy+1; front2x = cx+1; front2y = cy+1;
        back1x  = cx-1; back1y  = cy-1; back2x  = cx+1; back2y  = cy-1;
    } else if (stem_dir == 1) { //right
        front1x = cx+1; front1y = cy+1; front2x = cx+1; front2y = cy-1;
        back1x  = cx-1; back1y  = cy+1; back2x  = cx-1; back2y  = cy-1;
    } else if (stem_dir == 2) { //down
        front1x = cx-1; front1y = cy-1; front2x = cx+1; front2y = cy-1;
        back1x  = cx-1; back1y  = cy+1; back2x  = cx+1; back2y  = cy+1;
    } else if (stem_dir == 3) { //left
        front1x = cx-1; front1y = cy+1; front2x = cx-1; front2y = cy-1;
        back1x  = cx+1; back1y  = cy+1; back2x  = cx+1; back2y  = cy-1;
    } else {
        //no reliable stem found; fall back to 3-corner rule only -> treat as proper T-Spin
        return 2;
    }

    //count front/back corner occupancy (out-of-bounds counts as occupied)
    int front_count = 0;
    int back_count  = 0;

    if (front1x < 0 || front1x >= M_WIDTH || front1y < 0 || front1y >= M_HEIGHT) front_count++;
    else if (matrix[front1y][front1x] != EMPTY) front_count++;

    if (front2x < 0 || front2x >= M_WIDTH || front2y < 0 || front2y >= M_HEIGHT) front_count++;
    else if (matrix[front2y][front2x] != EMPTY) front_count++;

    if (back1x < 0 || back1x >= M_WIDTH || back1y < 0 || back1y >= M_HEIGHT) back_count++;
    else if (matrix[back1y][back1x] != EMPTY) back_count++;

    if (back2x < 0 || back2x >= M_WIDTH || back2y < 0 || back2y >= M_HEIGHT) back_count++;
    else if (matrix[back2y][back2x] != EMPTY) back_count++;

    //apply rules:
    //- if front_count == 2 and back_count >= 1 -> proper T-Spin
    if (front_count == 2 && back_count >= 1) return 2;

    //- else if front_count == 1 and back_count == 2 -> mini T-Spin
    if (front_count == 1 && back_count == 2) return 1;

    //- exception: if last rotation kick moved the center by a 1x2 offset, treat as proper T-spin
    if ((abs(last_rotation_dx) == 1 && abs(last_rotation_dy) == 2) ||
        (abs(last_rotation_dx) == 2 && abs(last_rotation_dy) == 1)) {
        return 2;
    }

    //otherwise, even though 3 corners are occupied, it does not match the front/back pattern -> no T-Spin
    return 0;
}

void update_clear() {
    lock_piece();

    int cleared = check_lines();
    total_lines_cleared += cleared;

    //determine T-Spin / Mini-T-Spin using placement and rotation flag
    int tspin_type = 0; //0 not tspin, 1 mini, 2 full
    if (active_piece.shape == T_PIECE && active_piece_was_rotated) {
        tspin_type = detect_tspin_from_placement(active_piece.x, active_piece.y);
    }

    //score and garbage calculation
    uint32_t base_score = 0;
    int lines_to_send = 0;
    bool qualifies_for_b2b = false;

    if (tspin_type > 0) {
        //T-Spin family
        if (tspin_type == 1) {
            //mini t-spin
            if (cleared == 0) {
                base_score = 100 * level; //mini no-line
                lines_to_send = 0;
                //mini no-line does not start B2B and does not get B2B bonus
                qualifies_for_b2b = false;
            } else if (cleared == 1) {
                base_score = 200 * level; //mini single
                lines_to_send = 0;
                qualifies_for_b2b = true;
            } else if (cleared == 2) {
                base_score = 400 * level;
                lines_to_send = 1;
                qualifies_for_b2b = true;
            } else {
                //unlikely: mini with >2 lines, treat as full t-spin line clear
                base_score = 1600 * level;
                lines_to_send = 6;
                qualifies_for_b2b = true;
            }
        } else {
            //full T-Spin
            if (cleared == 0) {
                base_score = 400 * level;
                lines_to_send = 0;
                qualifies_for_b2b = false;
            } else if (cleared == 1) {
                base_score = 800 * level;
                lines_to_send = 2;
                qualifies_for_b2b = true;
            } else if (cleared == 2) {
                base_score = 1200 * level;
                lines_to_send = 4;
                qualifies_for_b2b = true;
            } else if (cleared == 3) {
                base_score = 1600 * level;
                lines_to_send = 6;
                qualifies_for_b2b = true;
            }
        }
    } else {
        //not a T-Spin; handle normal line clears
        switch (cleared) {
            case 0:
                //no-line clear: receiver clears incoming garbage first
                if (multiplayer) add_garbage();
                break;
            case 1:
                base_score = 100 * level;
                lines_to_send = 0;
                break;
            case 2:
                base_score = 300 * level;
                lines_to_send = 1;
                break;
            case 3:
                base_score = 500 * level;
                lines_to_send = 2;
                break;
            case 4:
                base_score = 800 * level;
                lines_to_send = 4;
                qualifies_for_b2b = true;
                break;
        }
    }

    //combo handling
    if (cleared > 0) {
        if (combo_counter == -1) combo_counter = 0;
        else combo_counter++;

        //if combo streak > 0 (i.e., second consecutive clear), award combo garbage and a small combo score
        if (combo_counter > 0) {
            //send combo lines to opponent
            //if (multiplayer) lines_to_send += combo_counter;
            //add combo score: small bonus per combo (tunable)
            score += 50 * level * combo_counter;
        }
    } else {
        //reset combo streak on no line clear
        combo_counter = -1;
    }

    //back-to-back handling: if action qualifies and previous b2b active then apply bonus
    if (qualifies_for_b2b) {
        if (back_to_back_active) {
            //back-to-back bonus = +50% action total
            //we'll add base_score/2 extra (integer math)
            uint32_t b2b_bonus = base_score / 2;
            base_score += b2b_bonus;
            //send +1 extra garbage for back-to-back in multiplayer
            if (multiplayer) {
                lines_to_send += 1;

                if (tspin_type == 2 && cleared == 2) lines_to_send += 1;
                if (tspin_type == 2 && cleared == 3) lines_to_send += 2;
                if (cleared == 4) lines_to_send += 1;
            }
        }
        //set/continue back-to-back active
        back_to_back_active = true;
    } else {
        //t-spins without line clears do not break existing b2b; other actions do
        if (tspin_type > 0 && cleared == 0) {
            //do nothing, do not break b2b
        } else {
            //any non-qualifying action breaks b2b
            back_to_back_active = false;
        }
    }

    //perfect clear -> award 10 lines to opponent (send), not add_garbage
    if (perfect_clear_check()) {
        switch (cleared) {
            case 1:
                base_score += 800 * level;
                break;
            case 2:
                base_score += 1200 * level;
                break;
            case 3:
                base_score += 1800 * level;
                break;
            case 4:
                base_score += 2000 * level;
                break;
        }
    }

    //apply calculated score and send garbage if multiplayer
    score += base_score;

    if (multiplayer && perfect_clear_check() && cleared == 4) send_garbage(10);
    else if (multiplayer && lines_to_send > 0) send_garbage(lines_to_send);

    calculate_level();

    hold_avail = true;
    active_piece.shape = INACTIVE;
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

void mp_test() {
    int i = 0;
    received_ping = false;

    while (1) {
        if (received_ping) {
            matrix[i / M_WIDTH][i % M_WIDTH] = O_PIECE;
            i++;
        }

        received_ping = false;
        get_inputs();

        if (cur_inputs.rot_right) {
            int col;

            if (mp_handshake_blocking(250)) col = S_PIECE;
            else col = Z_PIECE;

            matrix[i / M_WIDTH][i % M_WIDTH] = col;
            i++;
        }

        render_frame();
        wait_and_push_frame();
    }
}

void pause_game() {
    if (!game_paused || game_paused == 3) {
        if (!game_paused) game_paused = 1;
        if (game_paused == 3) game_paused = 2;

        if (multiplayer && mp_pause_received == 0 && game_paused == 1) mp_send_msg_packed(mp_msg_pause, 0);
        else if (multiplayer && mp_pause_received == 1 && game_paused == 1) mp_pause_received = 0;

        game_paused_time = to_ms_since_boot(get_absolute_time());

        play_audio(PAUSE_SFX, true);
        song_paused = true;

        cancel_generation_timer();
        cancel_gravity();
        cancel_lock_timer();

        cur_sel = 0;
    } else {
        if (mp_pause_received == -1) {
            pause_game_exit:
            game_paused = 0;

            if (multiplayer && mp_pause_received == 0) mp_send_msg_packed(mp_msg_pause, 1);
            else if (multiplayer && mp_pause_received == -1) mp_pause_received = 0;

            game_start_time += to_ms_since_boot(get_absolute_time()) - game_paused_time;

            song_paused = false;

            return;
        }
    }

    render_frame();

    if (game_paused == 1) {
        int NUM_OPTS = 2;
        if (cur_inputs.up) {
            cur_sel = (cur_sel - 1 + NUM_OPTS) % NUM_OPTS;
            play_audio(SWITCH_OPTION_SFX, true);
        }

        if (cur_inputs.down) {
            cur_sel = (cur_sel + 1 + NUM_OPTS) % NUM_OPTS;
            play_audio(SWITCH_OPTION_SFX, true);
        }

        if (cur_inputs.pause || cur_inputs.b) {
            //consume
            cur_inputs.pause = false;
            cur_inputs.rot_left = false;
            cur_inputs.b = false;
            goto pause_game_exit;
        }

        if (cur_inputs.a) {        
            if (cur_sel == 0) {
                //consume
                cur_inputs.a = false;
                cur_inputs.rot_right = false;
                goto pause_game_exit;
            }

            if (cur_sel == 1) {
                play_audio(SELECT_OPTION_SFX, true);
                //todo fadeout
                if (multiplayer) mp_send_msg_packed(mp_msg_game_over, 2);
                game_over = 2;
                goto pause_game_exit;
            }
        }

        draw_text("paused", 32, 50, 0, S_PIECE);
        draw_text("resume", 32, 30, 0, cur_sel == 0 ? SELECTED_TEXT : UNSELECTED_TEXT);
        draw_text("quit game", 32, 22, 0, cur_sel == 1 ? SELECTED_TEXT : UNSELECTED_TEXT);
    }

    if (game_paused == 2) {
        if (raw_inputs.up && raw_inputs.b) {
            play_audio(SELECT_OPTION_SFX, true);
            //todo fadeout
            if (multiplayer) mp_send_msg_packed(mp_msg_game_over, 2);
            game_over = 2;
        }

        if (mp_sync_ready) {
            game_paused = 1;
        }

        draw_text("disconnected", 32, 50, 0, Z_PIECE);
        draw_text("press up+b", 32, 30, 0, UNSELECTED_TEXT);
        draw_text("to quit", 32, 24, 0, UNSELECTED_TEXT);
    }
}

void game_loop() {
    play_audio(SILENCE_SONG, false);
    reset_game();

    if (multiplayer) mp_drain_rx();

    render_frame();
    fade(250, 1);
    sleep_ms(100);

    if (multiplayer) {
        grav_opt_received = false;
        grav_opt_temp = 0;

        //send options sync
        mp_send_msg_packed(mp_msg_level_opt, start_level);
        mp_send_msg_packed(mp_msg_grav_opt, ((uint8_t)mp_level_timer) & 0x0F);
        mp_send_msg_packed(mp_msg_grav_opt_hi, ((uint8_t)mp_level_timer) & 0xF0);
    }

    play_audio(GAME_START_SFX, true);
    sleep_ms(3200);

    play_audio(song_choice, false);

    if (multiplayer && mp_test_en) mp_test();

    game_start_time = to_ms_since_boot(get_absolute_time());
    level = start_level_effective;

    mp_pause_received = 0;
    if (multiplayer) {
        mp_sync_awaiting = true;
        mp_sync_ready = true;
    }

    in_game = true;
    while (game_over == 0) {
        get_inputs();

        if (multiplayer && game_paused != 2 && !mp_sync_ready) {
            if (game_paused == 1) game_paused = 2;
            else game_paused = 3;
        }

        if (game_paused || cur_inputs.pause || mp_pause_received != 0) {
            //consume
            if (cur_inputs.pause && game_paused == 0) cur_inputs.pause = false;
            pause_game();

            if (game_over) break;
        }

        if (multiplayer) {
            mp_sync_ready = false;
            mp_send_msg_packed(mp_msg_ping, 2);
        }

        if (!game_paused) update_game();

        if (!game_paused) render_frame();
        wait_and_push_frame();
    }

    in_game = false;
    //game over
    cancel_generation_timer();
    cancel_gravity();
    cancel_lock_timer();
    mp_sync_awaiting = false;
    mp_sync_ready = false;
    mp_pause_received = 0;
    song_paused = false;

    final_time = (to_ms_since_boot(get_absolute_time()) - game_start_time) / 1000;
    
    if (game_over == 1) {
        if (multiplayer) mp_send_msg(mp_msg_game_over);
        play_audio(GAME_OVER_SFX, true);
        memset(matrix, Z_PIECE, sizeof(matrix));
    }

    if (game_over == -1) {
        play_audio(GAME_WIN_SFX, true);
        memset(matrix, S_PIECE, sizeof(matrix));
    }

    if (game_over == 2) {
        play_audio(SILENCE_SONG, false);
        cur_screen = title_screen;
        return;
    }

    active_piece.shape = INACTIVE;
    ghost_piece.shape = INACTIVE;

    play_audio(SILENCE_SONG, false);

    game_paused_time = to_ms_since_boot(get_absolute_time());
    render_frame();
    wait_and_push_frame();
    sleep_ms(1500);

    game_start_time += to_ms_since_boot(get_absolute_time()) - game_paused_time;
    render_frame();

    cur_screen = game_over_screen;
}