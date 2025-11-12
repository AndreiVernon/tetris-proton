#include "pico/stdlib.h"
#include "graphics.h"

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