#include "pico/stdlib.h"
#include <string.h>
#include "input.h"

#define DAS_FRAMES 18

typedef struct {
    bool up, down, left, right;
    bool a, b;
    bool select, start;
} RawInputState;

static RawInputState raw_inputs = {0};
InputState cur_inputs = {0};

static uint32_t left_buf, right_buf;
static bool last_move_dir; //0 = left, 1 = right
static uint8_t up_buf, a_buf, b_buf, start_buf, select_buf;

//init gpio and such
//set all buffer variables to 0
void init_inputs() {
    memset(&raw_inputs, 0, sizeof(raw_inputs));
    memset(&cur_inputs, 0, sizeof(cur_inputs));
    left_buf = right_buf = last_move_dir = up_buf = a_buf = b_buf = start_buf = select_buf = 0;

    //proton buttons
    gpio_init_mask(1 << 21 | 1 << 26);
    sio_hw->gpio_oe_clr = 1 << 21 | 1 << 26;
}

//set values in raw_inputs
void read_raw_inputs() {    
    raw_inputs.left = gpio_get(26);
    raw_inputs.right = gpio_get(21);
}


static inline bool edge_activated_u8(uint8_t buf) {
    //bit1 = previous, bit0 = current

    //00 - do nothing
    //01 - just pressed, activate input
    //11 - being held, do nothing
    //10 - just let go, do nothing 

    return (buf & 0b11u) == 0b01u;
}

//dir: 0 = left, 1 = right
static bool das_should_move(uint32_t* buf_p, bool alt, bool dir) {
    //buf LSB-first: consecutive trailing 1s = how many frames the button's been held (1..)

    //frame 1 : activate input
    //frames 2 to DAS_FRAMES : do nothing
    //frames >DAS_FRAMES : activate input every other frame

    uint32_t buf = *buf_p;

    //get the number of trailing 1s
    int hold_len = 0;
    uint32_t temp = buf;
    while (temp & 1u) {
        hold_len++; 
        temp >>= 1;
    }

    //not currently held, do nothing
    if (hold_len == 0) return false;

    //just pressed, activate input
    if (hold_len == 1) {
        //reset alt flag so that it's ready for next repeat
        *buf_p |= (1 << DAS_FRAMES);

        last_move_dir = dir;
        return true;
    }

    if (hold_len >= DAS_FRAMES) {
        //invert alt flag
        *buf_p |= ((alt ? 0 : 1) << DAS_FRAMES);
        return alt;
    }

    return false;
}

//three types of input
//1: no processing (just mirror the button state): soft drop
//2: hold and release (only activate after releasing and repressing): rotation, pause, hold, hard drop
//3: DAS (move once on press, delay 0.3s, then move every other frame): move left, move right
void get_inputs(void) {
    read_raw_inputs();

    //no processing
    cur_inputs.soft_drop = raw_inputs.down;

    //hold and release
    up_buf     = ((up_buf     << 1) | raw_inputs.up)     & 0b11;
    a_buf      = ((a_buf      << 1) | raw_inputs.a)      & 0b11;
    b_buf      = ((b_buf      << 1) | raw_inputs.b)      & 0b11;
    start_buf  = ((start_buf  << 1) | raw_inputs.start)  & 0b11;
    select_buf = ((select_buf << 1) | raw_inputs.select) & 0b11;

    cur_inputs.rot_left  = edge_activated_u8(a_buf);
    cur_inputs.rot_right = edge_activated_u8(b_buf);
    cur_inputs.hard_drop = edge_activated_u8(up_buf);
    cur_inputs.hold      = edge_activated_u8(start_buf);
    cur_inputs.pause     = edge_activated_u8(select_buf);

    //if both rot_left and rot_right are active, default to left
    if (cur_inputs.rot_left && cur_inputs.rot_right) cur_inputs.rot_right = false;

    //DAS
    bool alt = (left_buf >> DAS_FRAMES) & 1u; //track every-other-frame movement
    left_buf  = ((left_buf  << 1) | (raw_inputs.left)) & ((1u << DAS_FRAMES) - 1u);
    cur_inputs.left = das_should_move(&left_buf, alt, false);

    alt = (right_buf >> DAS_FRAMES) & 1u;
    right_buf = ((right_buf << 1) | (raw_inputs.right)) & ((1u << DAS_FRAMES) - 1u);
    cur_inputs.right = das_should_move(&right_buf, alt, true);

    //if both directions are pressed, only enable most recent
    if (cur_inputs.left && cur_inputs.right) {
        if (!last_move_dir) cur_inputs.right = false;
        else cur_inputs.left = false;
    }
}