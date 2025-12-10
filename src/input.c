#include "pico/stdlib.h"
#include <string.h>
#include "input.h"

#define DAS_FRAMES 10

#define CLK_PIN 18
#define LAT_PIN 19
#define D0_PIN 20

RawInputState raw_inputs = {0};
InputState cur_inputs = {0};

static uint64_t left_press_time, left_last_rep;
static uint64_t right_press_time, right_last_rep;
static bool last_move_dir; //0 = left, 1 = right
static uint8_t up_buf, down_buf, left_edge_buf, right_edge_buf, a_buf, b_buf, start_buf, select_buf;

//das timings in microseconds
//10 frames ~= 167ms
//2 frames  ~= 33ms
//subtract 2ms to account for frame times
#define DAS_US 164000ULL
#define ARR_US 31000ULL

//init gpio and such
//set all buffer variables to 0
void init_inputs() {
    memset(&raw_inputs, 0, sizeof(raw_inputs));
    memset(&cur_inputs, 0, sizeof(cur_inputs));

    left_press_time = right_press_time = 0ULL;
    left_last_rep   = right_last_rep   = 0ULL;
    last_move_dir = false;
    up_buf = down_buf = left_edge_buf = right_edge_buf = a_buf = b_buf = start_buf = select_buf = 0;

    // controller gpio
    gpio_init_mask(0b111 << 18);
    gpio_set_dir(18, true);
    gpio_set_dir(19, true);
    gpio_set_dir(20, false);
}

//set values in raw_inputs
void read_raw_inputs() {
    gpio_put(LAT_PIN, 1);
    sleep_us(12);
    gpio_put(LAT_PIN, 0);

    bool buttons_array[8];
    buttons_array[0] = !gpio_get(D0_PIN);
    sleep_us(6);

    for(int i = 1; i < 8; i++) {
        gpio_put(CLK_PIN, 1);
        sleep_us(6);
        gpio_put(CLK_PIN, 0);

        buttons_array[i] = !gpio_get(D0_PIN);
        sleep_us(6);
    }

    gpio_put(CLK_PIN, 1);
    sleep_us(6);
    gpio_put(CLK_PIN, 0);

    raw_inputs.a = buttons_array[0];
    raw_inputs.b = buttons_array[1];
    raw_inputs.select = buttons_array[2];
    raw_inputs.start = buttons_array[3];
    raw_inputs.up = buttons_array[4];
    raw_inputs.down = buttons_array[5];
    raw_inputs.left = buttons_array[6];
    raw_inputs.right = buttons_array[7];
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
static bool das_should_move(bool curr_pressed, uint64_t *press_time_p, uint64_t *last_rep_p, bool dir) {
    //if not held, reset timers and do nothing
    if (!curr_pressed) {
        *press_time_p = 0ULL;
        *last_rep_p = 0ULL;
        return false;
    }

    uint64_t now = to_us_since_boot(get_absolute_time());

    //just pressed: press_time was zero
    if (*press_time_p == 0ULL) {
        *press_time_p = now;
        *last_rep_p = now;
        last_move_dir = dir;
        return true;
    }

    //held but still in initial DAS delay
    if (now - *press_time_p < DAS_US) {
        return false;
    }

    //past DAS delay: allow repeats at ARR_US interval
    if (now - *last_rep_p >= ARR_US) {
        *last_rep_p = now;
        last_move_dir = dir;
        return true;
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
    cur_inputs.soft_drop = raw_inputs.up;

    //hold and release
    up_buf         = ((up_buf         << 1) | raw_inputs.up)   & 0b11;
    down_buf       = ((down_buf       << 1) | raw_inputs.down)   & 0b11;
    left_edge_buf  = ((left_edge_buf  << 1) | raw_inputs.left)   & 0b11;
    right_edge_buf = ((right_edge_buf << 1) | raw_inputs.right)   & 0b11;
    a_buf          = ((a_buf          << 1) | raw_inputs.a)      & 0b11;
    b_buf          = ((b_buf          << 1) | raw_inputs.b)      & 0b11;
    start_buf      = ((start_buf      << 1) | raw_inputs.start)  & 0b11;
    select_buf     = ((select_buf     << 1) | raw_inputs.select) & 0b11;

    cur_inputs.rot_left  = edge_activated_u8(b_buf);
    cur_inputs.rot_right = edge_activated_u8(a_buf);
    cur_inputs.hard_drop = edge_activated_u8(down_buf);
    cur_inputs.hold      = edge_activated_u8(start_buf);
    cur_inputs.pause     = edge_activated_u8(select_buf);

    //if both rot_left and rot_right are active, default to left
    if (cur_inputs.rot_left && cur_inputs.rot_right) cur_inputs.rot_right = false;

    //DAS (timer-based)
    cur_inputs.left  = das_should_move(raw_inputs.left,  &left_press_time,  &left_last_rep,  false);
    cur_inputs.right = das_should_move(raw_inputs.right, &right_press_time, &right_last_rep, true);

    //if both directions are pressed, only enable most recent
    if (cur_inputs.left && cur_inputs.right) {
        if (!last_move_dir) cur_inputs.right = false;
        else cur_inputs.left = false;
    }

    //mirror menu inputs
    cur_inputs.up = edge_activated_u8(up_buf);
    cur_inputs.down = cur_inputs.hard_drop;
    cur_inputs.left_edge = edge_activated_u8(left_edge_buf);
    cur_inputs.right_edge = edge_activated_u8(right_edge_buf);
    cur_inputs.a = cur_inputs.rot_right;
    cur_inputs.b = cur_inputs.rot_left;
    cur_inputs.start = cur_inputs.hold;
    cur_inputs.select = cur_inputs.pause;
}
