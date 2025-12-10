#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hub75.pio.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include <string.h>
#include "display.h"

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

//assigning GPIOs to data cable ports for adafruit display ---> adjust numbers as needed
#define DATA_BASE_PIN 2
#define R1 (DATA_BASE_PIN + 0)   //red data for top half
#define G1 (DATA_BASE_PIN + 1)   //green data for top half
#define B1 (DATA_BASE_PIN + 2)   //blue data for top half
#define R2 (DATA_BASE_PIN + 3)   //red data for bottom half
#define G2 (DATA_BASE_PIN + 4)   //green data for bottom half
#define B2 (DATA_BASE_PIN + 5)   //blue data for bottom half
#define A 13    //row select bit 0
#define B 10    //row select bit 1
#define C 8    //row select bit 2
#define D 11    //row select bit 3
#define E 9    //row select bit 4
#define CLK 14  //clock (shift register)
#define LAT 12  //stores shifted data into output register --> latch
#define OE 15   //active low output enable

#define GPIO_MASK ((1u<<R1) | (1u<<G1) | (1u<<B1) | (1u<<R2) | (1u<<G2) | (1u<<B2) | (1u<<A) | (1u<<B) | (1u<<C) | (1u<<D) | (1u<<E) | (1u<<CLK) | (1u<<LAT) | (1u<<OE))


//https://github.com/hzeller/rpi-rgb-led-matrix

//BCM configuration (tunable)
#define BCM_BITS 5       //number of bit planes (3 => uses bits 7,6,5 of each color)
#define LSB_TIME_US 2    //duration of LSB plane in microseconds (tweak if flicker)

//framebuffer[id][row][col][rgb] - 3d array that stores what to display
uint8_t framebuffer[2][PANEL_HEIGHT][PANEL_WIDTH][3];
//which framebuffer is ready to present
volatile bool fbf_rdy;
volatile bool fbf_swap_request = false;

//masks for fast operations
static const uint32_t DATA_MASK = (1u<<R1)|(1u<<G1)|(1u<<B1)|(1u<<R2)|(1u<<G2)|(1u<<B2);
static const uint32_t ADDR_MASK = (1u<<A)|(1u<<B)|(1u<<C)|(1u<<D)|(1u<<E);

void display_clear() {
    memset(framebuffer, 0, sizeof(framebuffer));
}

// initialize GPIOs
void display_init() {
    int pins[] = {R1, G1, B1, R2, G2, B2, A, B, C, D, E, CLK, LAT, OE};
    for (int i = 0; i < (int)(sizeof(pins)/sizeof(pins[0])); ++i) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_OUT);
        gpio_put(pins[i], 0);
    }

    //ensure outputs disabled to start (OE is active low)
    gpio_put(OE, 1);
    gpio_put(LAT, 0);
    gpio_put(CLK, 0);

    display_clear();
    fbf_rdy = 0;
}

//set the row address lines A..E for a half-row index [0..31]
static inline void send_row_address(uint8_t row) {
    //produce mask for address lines (atomic)
    uint32_t mask_set = 0;
    if (row & 0x01) mask_set |= (1u<<A);
    if (row & 0x02) mask_set |= (1u<<B);
    if (row & 0x04) mask_set |= (1u<<C);
    if (row & 0x08) mask_set |= (1u<<D);
    if (row & 0x10) mask_set |= (1u<<E);

    //clear address bits then set as needed
    gpio_clr_mask(ADDR_MASK);
    gpio_set_mask(mask_set);
}

//shift one half-row (64 columns) for given row and bit plane using bit-banging
static void shift_row_bitplane(uint8_t row, uint8_t bit_plane) {
    //we scan half-rows: top = row, bottom = row + 32
    const uint8_t top_row = (PANEL_HEIGHT - 1) - row;
    const uint8_t bot_row = (PANEL_HEIGHT/2 - 1) - row;

    //choose which bit of each 8-bit channel to use
    const int bit_index = 7 - bit_plane; // bit_plane 0 => MSB (bit 7)

    //disable outputs while shifting
    gpio_put(OE, 1);

    //set address lines for this row
    send_row_address(row);

    //for every column: set data lines (R1,G1,B1,R2,G2,B2) then pulse CLK
    for (int col = 0; col < PANEL_WIDTH; ++col) {
        uint32_t set_mask = 0;
        //top half pixel at (top_row, col)
        uint8_t r1 = framebuffer[fbf_rdy][top_row][col][0];
        uint8_t g1 = framebuffer[fbf_rdy][top_row][col][1];
        uint8_t b1 = framebuffer[fbf_rdy][top_row][col][2];

        //bottom half pixel at (bot_row, col)
        uint8_t r2 = framebuffer[fbf_rdy][bot_row][col][0];
        uint8_t g2 = framebuffer[fbf_rdy][bot_row][col][1];
        uint8_t b2 = framebuffer[fbf_rdy][bot_row][col][2];

        if (r1 & (1u << bit_index)) set_mask |= (1u << R1);
        if (g1 & (1u << bit_index)) set_mask |= (1u << G1);
        if (b1 & (1u << bit_index)) set_mask |= (1u << B1);

        if (r2 & (1u << bit_index)) set_mask |= (1u << R2);
        if (g2 & (1u << bit_index)) set_mask |= (1u << G2);
        if (b2 & (1u << bit_index)) set_mask |= (1u << B2);

        //write the data lines atomically: clear then set
        gpio_clr_mask(DATA_MASK);
        if (set_mask) gpio_set_mask(set_mask);

        //pulse clock: rising edge shifts the data into the panel's shift registers
        gpio_put(CLK, 1);
        gpio_put(CLK, 0);
    }

    // after shifting all columns, latch the row data into outputs
    gpio_put(LAT, 1);

    // short hold to meet panel latch timings
    //sleep_us(1);
    busy_wait_at_least_cycles(50 * 0.3); //150 cycles = 1us

    gpio_put(LAT, 0);

    // compute display time weight for this bit plane (LSB -> shortest)
    uint32_t weight_us = (uint32_t)LSB_TIME_US << (BCM_BITS - 1 - bit_plane);

    // enable outputs for that duration
    gpio_put(OE, 0);
    sleep_us(weight_us);
    gpio_put(OE, 1);
}

//top-level refresh: iterate over bitplanes, then half-rows 0..31
void display_refresh() {
    // For each bit-plane (MSB first)
    for (uint8_t bit_plane = 0; bit_plane < BCM_BITS; bit_plane++) {
        //scan half-rows (0..(PANEL_HEIGHT/2 - 1))
        //doing even rows then odd rows in interlaced pattern reduces perceived flickering
        for (uint8_t row = 0; row < (PANEL_HEIGHT / 2); row += 2) {
            shift_row_bitplane(row, bit_plane);
        }
        for (uint8_t row = 1; row < (PANEL_HEIGHT / 2); row += 2) {
            shift_row_bitplane(row, bit_plane);
        }
        // for (uint8_t row = 0; row < (PANEL_HEIGHT / 2); row++) {
        //     shift_row_bitplane(row, bit_plane);
        // }
    }
}

// pixel helpers
void display_set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= PANEL_WIDTH || y >= PANEL_HEIGHT) return;
    framebuffer[!fbf_rdy][y][x][0] = r;
    framebuffer[!fbf_rdy][y][x][1] = g;
    framebuffer[!fbf_rdy][y][x][2] = b;
}

void display_fill(uint8_t r, uint8_t g, uint8_t b) {
    for (int y = 0; y < PANEL_HEIGHT; ++y)
        for (int x = 0; x < PANEL_WIDTH; ++x)
            display_set_pixel(x, y, r, g, b);
}

// example test pattern (kept / adjusted)
void display_test_pattern() {
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

void pio_loop() {
    PIO pio = pio0;
    int sm_data = 0;

    uint offset = pio_add_program(pio, &hub75_data_rgb888_program);

    /*configure pins for pio SM*/
    for (uint i = DATA_BASE_PIN; i < DATA_BASE_PIN + 6; ++i) pio_gpio_init(pio, i);
    pio_gpio_init(pio, CLK);

    pio_sm_config c = hub75_data_rgb888_program_get_default_config(offset);
    sm_config_set_out_pins(&c, DATA_BASE_PIN, 6);
    sm_config_set_sideset_pins(&c, CLK);
    /*tx fifo joined tx (32-bit pushes)*/
    sm_config_set_out_shift(&c, true, true, 24);
    sm_config_set_in_shift(&c, false, false, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    pio_sm_init(pio, sm_data, offset, &c);
    /*start entry point so SM cycles through wrap forever*/
    pio_sm_exec(pio, sm_data, offset + hub75_data_rgb888_offset_entry_point);
    pio_sm_set_enabled(pio, sm_data, true);

    /*temp per-row buffers (top/bottom lanes)*/
    static uint32_t rowbuf[2][PANEL_WIDTH];

    /*main refresh loop (cpu drives row+lat+oe, pio consumes pixel words)
      this mirrors display_refresh: iterate bit-planes MSB-first and use even/odd
      half-row ordering to reduce perceived flicker. no conversions; framebuffer
      format is assumed identical (8-bit r,g,b per channel). */
    while (1) {
        /*for each bit-plane (msb first)*/
        for (uint8_t bit_plane = 0; bit_plane < BCM_BITS; ++bit_plane) {
            /*translate to preshift amount used by the pio program (keep same mapping
              as original code: bit = 7 - bit_plane) */
            uint8_t bit = 7 - bit_plane;

            /*patch the pio preshift instruction once per bit-plane*/
            uint16_t instr;
            if (bit == 0) instr = pio_encode_pull(false, true);
            else instr = pio_encode_out(pio_null, bit);
            pio->instr_mem[offset + hub75_data_rgb888_offset_shift0] = instr;
            pio->instr_mem[offset + hub75_data_rgb888_offset_shift1] = instr;

            /*scan half-rows in same even-then-odd order as display_refresh*/
            for (uint8_t start = 0; start < 2; ++start) {
                for (uint8_t rowsel = start; rowsel < (PANEL_HEIGHT / 2); rowsel += 2) {

                    /*build top & bottom 24-bit per-column words directly from framebuffer
                      format: 0xRRGGBB (no gamma / no 565 conversion) */
                    const int top_base = (PANEL_HEIGHT - 1) - rowsel;
                    const int bot_base = (PANEL_HEIGHT / 2 - 1) - rowsel;

                    for (int x = 0; x < PANEL_WIDTH; ++x) {
                        uint8_t r1 = framebuffer[fbf_rdy][top_base][x][0];
                        uint8_t g1 = framebuffer[fbf_rdy][top_base][x][1];
                        uint8_t b1 = framebuffer[fbf_rdy][top_base][x][2];

                        uint8_t r2 = framebuffer[fbf_rdy][bot_base][x][0];
                        uint8_t g2 = framebuffer[fbf_rdy][bot_base][x][1];
                        uint8_t b2 = framebuffer[fbf_rdy][bot_base][x][2];

                        rowbuf[0][x] = ((uint32_t)r1 << 16) | ((uint32_t)g1 << 8) | (uint32_t)b1;
                        rowbuf[1][x] = ((uint32_t)r2 << 16) | ((uint32_t)g2 << 8) | (uint32_t)b2;
                    }

                    /*push pixels: two 32-bit words per column (top lane, bottom lane)*/
                    for (int x = 0; x < PANEL_WIDTH; ++x) {
                        pio_sm_put_blocking(pio, sm_data, rowbuf[0][x]);
                        pio_sm_put_blocking(pio, sm_data, rowbuf[1][x]);
                    }

                    /*dummy pixels to clock final bits through pio (one per lane)*/
                    pio_sm_put_blocking(pio, sm_data, 0);
                    pio_sm_put_blocking(pio, sm_data, 0);

                    /*wait for SM to finish (stall on empty TX FIFO)*/
                    uint32_t txstall_mask = 1u << (PIO_FDEBUG_TXSTALL_LSB + sm_data);
                    pio->fdebug = txstall_mask;
                    while (!(pio->fdebug & txstall_mask)) tight_loop_contents();

                    /*set row address*/
                    send_row_address(rowsel);

                    /*pulse latch*/
                    gpio_put(LAT, 1);
                    busy_wait_at_least_cycles(150 * 0.3);
                    gpio_put(LAT, 0);

                    /*compute OE pulse width for this bit plane (mirror display_refresh)*/
                    uint32_t weight_us = (uint32_t)LSB_TIME_US << (BCM_BITS - 1 - bit_plane);

                    /*enable outputs (active low) for the plane duration*/
                    gpio_put(OE, 0);
                    sleep_us(weight_us);
                    gpio_put(OE, 1);
                }
            }
        } /*end bit-plane loop*/

        /*allow swap if requested (mirror display_loop: swap once per full refresh)*/
        if (fbf_swap_request) {
            fbf_rdy = !fbf_rdy;
            fbf_swap_request = false;
        }
    } /*end while*/
}


void display_loop() {
    while (true) {
        display_refresh();
        
        if (fbf_swap_request) {
            fbf_rdy = !fbf_rdy;
            fbf_swap_request = false;
        }
    }
}