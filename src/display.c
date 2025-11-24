#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include <string.h>
#include "display.h"
#include "assets.h"

// 64x64 RGB LED Matrix - 3mm Pitch - 192mm x 192mm; Product ID: 4732
// https://www.adafruit.com/product/4732
// ------------------------------------------------------------------------
// correct pinout
// R1 [x x] G1
// B1 [x x] GND
// R2 [x x] G2
// B2 [x x] E
// A  [x x] B
// C  [x x] D
// CLK[x x] LAT
// OE [x x] GND

// https://docs.cirkitdesigner.com/component/885af448-2bdb-49bc-ae1b-0e781522c801/hub75
// ------------------------------------------------------------------------

// assigning GPIOs to data cable ports for adafruit display ---> adjust numbers as needed
#define R1 16 // red data for top half
#define G1 10 // green data for top half
#define B1 17 // blue data for top half
#define R2 18 // red data for bottom half
#define G2 11 // green data for bottom half
#define B2 19 // blue data for bottom half
#define A 20 // row select bit 0
#define B 13 // row select bit 1
#define C 21 // row select bit 2
#define D 14 // row select bit 3
#define E 12 // row select bit 4
#define CLK 22 // clock (shift register)
#define LAT 15 // stores shifted data into output register --> latch
#define OE 23 // active low output enable

#define GPIO_MASK ((1u<<R1) | (1u<<G1) | (1u<<B1) | (1u<<R2) | (1u<<G2) | (1u<<B2) | (1u<<A) | (1u<<B) | (1u<<C) | (1u<<D) | (1u<<E) | (1u<<CLK) | (1u<<LAT) | (1u<<OE))


// https://github.com/hzeller/rpi-rgb-led-matrix

// BCM configuration (tunable)
#define BCM_BITS 6        // number of bit planes (3 => uses bits 7,6,5 of each color)
#define LSB_TIME_US 1    // duration of LSB plane in microseconds (tweak if flicker)

// framebuffer[row][col][rgb] - 3d array that stores what to display
uint8_t framebuffer[PANEL_HEIGHT][PANEL_WIDTH][3];

// masks for fast operations
static const uint32_t DATA_MASK = (1u<<R1)|(1u<<G1)|(1u<<B1)|(1u<<R2)|(1u<<G2)|(1u<<B2);
static const uint32_t ADDR_MASK = (1u<<A)|(1u<<B)|(1u<<C)|(1u<<D)|(1u<<E);

// initialize GPIOs
void display_init()
{
    int pins[] = {R1, G1, B1, R2, G2, B2, A, B, C, D, E, CLK, LAT, OE};
    for (int i = 0; i < (int)(sizeof(pins)/sizeof(pins[0])); ++i) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_OUT);
        gpio_put(pins[i], 0);
    }
    // ensure outputs disabled to start (OE is active low)
    gpio_put(OE, 1);
    gpio_put(LAT, 0);
    gpio_put(CLK, 0);

    // clear framebuffer if desired
    memset(framebuffer, 0, sizeof(framebuffer));
    sleep_ms(10);
}

// set the row address lines A..E for a half-row index [0..31]
static inline void send_row_address(uint8_t row)
{
    // produce mask for address lines (atomic)
    uint32_t mask_set = 0;
    if (row & 0x01) mask_set |= (1u<<A);
    if (row & 0x02) mask_set |= (1u<<B);
    if (row & 0x04) mask_set |= (1u<<C);
    if (row & 0x08) mask_set |= (1u<<D);
    if (row & 0x10) mask_set |= (1u<<E);

    // clear address bits then set as needed
    gpio_clr_mask(ADDR_MASK);
    gpio_set_mask(mask_set);
}

// shift one half-row (64 columns) for given row and bit plane using bit-banging
static void shift_row_bitplane(uint8_t row, uint8_t bit_plane)
{
    // we scan half-rows: top = row, bottom = row + 32
    const uint8_t top_row = (PANEL_HEIGHT - 1) - row;
    const uint8_t bot_row = (PANEL_HEIGHT/2 - 1) - row;

    // choose which bit of each 8-bit channel to use
    const int bit_index = 7 - bit_plane; // bit_plane 0 => MSB (bit 7)

    // disable outputs while shifting
    gpio_put(OE, 1);

    // set address lines for this row
    send_row_address(row);

    // For every column: set data lines (R1,G1,B1,R2,G2,B2) then pulse CLK
    for (int col = 0; col < PANEL_WIDTH; ++col)
    {
        uint32_t set_mask = 0;
        // top half pixel at (top_row, col)
        uint8_t r1 = framebuffer[top_row][col][0];
        uint8_t g1 = framebuffer[top_row][col][1];
        uint8_t b1 = framebuffer[top_row][col][2];

        // bottom half pixel at (bot_row, col)
        uint8_t r2 = framebuffer[bot_row][col][0];
        uint8_t g2 = framebuffer[bot_row][col][1];
        uint8_t b2 = framebuffer[bot_row][col][2];

        if (r1 & (1u << bit_index)) set_mask |= (1u << R1);
        if (g1 & (1u << bit_index)) set_mask |= (1u << G1);
        if (b1 & (1u << bit_index)) set_mask |= (1u << B1);

        if (r2 & (1u << bit_index)) set_mask |= (1u << R2);
        if (g2 & (1u << bit_index)) set_mask |= (1u << G2);
        if (b2 & (1u << bit_index)) set_mask |= (1u << B2);

        // write the data lines atomically: clear then set
        gpio_clr_mask(DATA_MASK);
        if (set_mask) gpio_set_mask(set_mask);

        // pulse clock: rising edge shifts the data into the panel's shift registers
        gpio_put(CLK, 1);
        gpio_put(CLK, 0);
    }

    // after shifting all columns, latch the row data into outputs
    gpio_put(LAT, 1);
    // short hold to meet panel latch timings
    sleep_us(1);
    gpio_put(LAT, 0);

    // compute display time weight for this bit plane (LSB -> shortest)
    uint32_t weight_us = (uint32_t)LSB_TIME_US << (BCM_BITS - 1 - bit_plane);

    // enable outputs for that duration
    gpio_put(OE, 0);
    sleep_us(weight_us);
    gpio_put(OE, 1);
}

// top-level refresh: iterate over bitplanes, then half-rows 0..31
void display_refresh()
{
    // For each bit-plane (MSB first)
    for (uint8_t bit_plane = 0; bit_plane < BCM_BITS; ++bit_plane)
    {
        // scan half-rows (0..(PANEL_HEIGHT/2 - 1))
        for (uint8_t row = 0; row < (PANEL_HEIGHT / 2); ++row)
        {
            shift_row_bitplane(row, bit_plane);
        }
    }
}

// pixel helpers
void display_set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= PANEL_WIDTH || y >= PANEL_HEIGHT) return;
    framebuffer[y][x][0] = r;
    framebuffer[y][x][1] = g;
    framebuffer[y][x][2] = b;
}

void display_clear()
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

void display_fill(uint8_t r, uint8_t g, uint8_t b)
{
    for (int y = 0; y < PANEL_HEIGHT; ++y)
        for (int x = 0; x < PANEL_WIDTH; ++x)
            display_set_pixel(x, y, r, g, b);
}

// example test pattern (kept / adjusted)
void display_test_pattern()
{
    display_fill(0,0,0);
    // quadrants using framebuffer coordinates where (0,0) is top-left here
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) display_set_pixel(x, y, 255, 0, 0);        // top-left red
        for (int x = 32; x < 64; ++x) display_set_pixel(x, y, 0, 255, 0);       // top-right green
    }
    for (int y = 32; y < 64; ++y) {
        for (int x = 0; x < 32; ++x) display_set_pixel(x, y, 0, 0, 255);        // bottom-left blue
        for (int x = 32; x < 64; ++x) display_set_pixel(x, y, 255, 255, 0);    // bottom-right yellow
    }
}

void display_loop() {
    stdio_init_all();
    display_init();

    // Ensure background/asset copy size is correct:
    memcpy(framebuffer, background, sizeof(framebuffer)); // background must match framebuffer size

    while (1) {
        display_refresh();
        // do other non-blocking tasks here if needed
    }
}