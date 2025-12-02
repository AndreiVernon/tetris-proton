#include <stdio.h>
#include "pico/stdlib.h"
#include <hardware/gpio.h>

typedef struct {
    bool up, down, left, right;
    bool a, b;
    bool select, start;
} RawInputState;

static RawInputState raw_inputs = {0};
uint64_t target;

int cont_clk_pin = 18;
int cont_lat_pin = 19;
int cont_d0_pin = 20;

// initalize pins
void init_pins() {
    gpio_init_mask(0b111<<18);
    gpio_set_dir(18 , true);
    gpio_set_dir(19 , true);
    gpio_set_dir(20 , false);
}

// initialize timer0
void init_timer() {
    hw_set_bits(&timer_hw->inte, 1u << 0);
    irq_set_exclusive_handler(TIMER0_IRQ_0, controller_isr);
    irq_set_enabled(TIMER0_IRQ_0, true);
    
}

void controller_isr() {
    timer_hw->intr = 1<<0;
    sio_hw->gpio_togl = 1<<(cont_lat_pin);
    sleep_us(12);
    sio_hw->gpio_togl = 1<<(cont_lat_pin);
    bool buttons_array[8];
    for(int i = 0 ; i < 8 ; i++) {
        sleep_us(6);
        sio_hw->gpio_togl = 1<<(cont_clk_pin);
        buttons_array[i] = !(sio_hw->gpio_out | (1<<cont_d0_pin));
        sleep_us(6);
        sio_hw->gpio_togl = 1<<(cont_clk_pin);
    }
    raw_inputs.a = buttons_array[0];
    raw_inputs.b = buttons_array[1];
    raw_inputs.select = buttons_array[2];
    raw_inputs.start = buttons_array[3];
    raw_inputs.up = buttons_array[4];
    raw_inputs.down = buttons_array[5];
    raw_inputs.left = buttons_array[6];
    raw_inputs.right = buttons_array[7];
    target += (1/60) * 1000000;
    timer_hw->alarm[0] = (uint32_t) target;
}