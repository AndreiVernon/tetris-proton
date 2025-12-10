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

uint32_t scanline_buffer[PANEL_WIDTH];

// PIO and DMA globals
PIO pio_hub75 = pio0;
uint sm_hub75 = 0;
uint dma_chan_hub75 = 0;

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



void display_loop() {
    while (true) {
        display_refresh();
        
        if (fbf_swap_request) {
            fbf_rdy = !fbf_rdy;
            fbf_swap_request = false;
        }
    }
}

void hub75_pio_init() {
    // Find a free state machine
    uint offset = pio_add_program(pio_hub75, &hub75_program);
    sm_hub75 = pio_claim_unused_sm(pio_hub75, true);
    
    // Configure PIO
    pio_sm_config c = hub75_program_get_default_config(offset);
    
    // 1. Configure SIDESET for CLK (Pin 14)
    sm_config_set_sideset_pins(&c, CLK);
    pio_gpio_init(pio_hub75, CLK);
    pio_sm_set_consecutive_pindirs(pio_hub75, sm_hub75, CLK, 1, true);

    // 2. Configure OUT pins for Data (Base Pin 2, Count 6: R1..B2)
    // Note: This relies on R1, G1, B1, R2, G2, B2 being contiguous starting at DATA_BASE_PIN
    sm_config_set_out_pins(&c, DATA_BASE_PIN, 6);
    for(int i=0; i<6; i++) {
        pio_gpio_init(pio_hub75, DATA_BASE_PIN + i);
    }
    pio_sm_set_consecutive_pindirs(pio_hub75, sm_hub75, DATA_BASE_PIN, 6, true);

    // 3. Setup FIFO (Auto-pull threshold 32 is fine, but we only need 6 bits per shift)
    // We will shift out 6 bits, shift_right=true, autopull=true, pull_threshold=anything >=6
    sm_config_set_out_shift(&c, true, true, 32); 
    
    // 4. Set Clock Divider
    // Target roughly 10-20MHz pixel clock. RP2350 is fast, let's divide comfortably.
    // 150MHz / 8 = 18.75MHz. Adjust 'div' if panel flickers or glitches.
    float div = clock_get_hz(clk_sys) / 20000000.0f; 
    sm_config_set_clkdiv(&c, div);

    // Initialize and enable
    pio_sm_init(pio_hub75, sm_hub75, offset, &c);
    pio_sm_set_enabled(pio_hub75, sm_hub75, true);
}

void hub75_dma_init() {
    dma_chan_hub75 = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan_hub75);
    
    // Transfer 32-bit words (our scanline buffer)
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    // Increment read address (buffer), fixed write address (PIO FIFO)
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    // DREQ based on PIO TX FIFO
    channel_config_set_dreq(&c, pio_get_dreq(pio_hub75, sm_hub75, true));

    dma_channel_configure(
        dma_chan_hub75,
        &c,
        &pio_hub75->txf[sm_hub75], // Write address
        NULL,                      // Read address (set later)
        PANEL_WIDTH,               // Transfer count
        false                      // Don't start yet
    );
}

void pio_init() {
    display_init();
    hub75_pio_init();
    hub75_dma_init();
}

// Replaces display_loop with PIO+DMA implementation
void pio_loop() {
    // 1. Initialize Hardware
    // (Assuming gpio_init for A,B,C,D,E,LAT,OE was done in main or display_init)
    hub75_pio_init();
    hub75_dma_init();

    while (true) {
        // --- Refresh Logic (Previously display_refresh) ---
        
        // Iterate over bit planes (MSB first)
        for (uint8_t bit_plane = 0; bit_plane < BCM_BITS; bit_plane++) {
            // Determine the bit index we are extracting (0..7)
            const int bit_index = 7 - bit_plane;
            const uint32_t bit_mask = (1u << bit_index);

            // Iterate over interlaced rows (0..15 then 1..15)
            // We use the exact same interlacing logic as your original code
            for (int pass = 0; pass < 2; pass++) {
                for (uint8_t row = pass; row < (PANEL_HEIGHT / 2); row += 2) {
                    
                    // --- 1. Prepare Buffer (Bit Slicing) ---
                    // This replaces the nested loop inside shift_row_bitplane
                    
                    // Logic from original: top = (H-1)-row, bot = (H/2-1)-row
                    const uint8_t top_row = (PANEL_HEIGHT - 1) - row;
                    const uint8_t bot_row = (PANEL_HEIGHT/2 - 1) - row;

                    for (int col = 0; col < PANEL_WIDTH; ++col) {
                        uint32_t pixel_bits = 0;

                        // Retrieve Top Pixel
                        uint8_t r1 = framebuffer[fbf_rdy][top_row][col][0];
                        uint8_t g1 = framebuffer[fbf_rdy][top_row][col][1];
                        uint8_t b1 = framebuffer[fbf_rdy][top_row][col][2];

                        // Retrieve Bottom Pixel
                        uint8_t r2 = framebuffer[fbf_rdy][bot_row][col][0];
                        uint8_t g2 = framebuffer[fbf_rdy][bot_row][col][1];
                        uint8_t b2 = framebuffer[fbf_rdy][bot_row][col][2];

                        // Pack bits into positions 0..5 (matching R1..B2 pin order)
                        // R1=bit0, G1=bit1, B1=bit2, R2=bit3, G2=bit4, B2=bit5
                        if (r1 & bit_mask) pixel_bits |= (1u << 0); // R1
                        if (g1 & bit_mask) pixel_bits |= (1u << 1); // G1
                        if (b1 & bit_mask) pixel_bits |= (1u << 2); // B1
                        
                        if (r2 & bit_mask) pixel_bits |= (1u << 3); // R2
                        if (g2 & bit_mask) pixel_bits |= (1u << 4); // G2
                        if (b2 & bit_mask) pixel_bits |= (1u << 5); // B2

                        scanline_buffer[col] = pixel_bits;
                    }

                    // --- 2. Send Row Address ---
                    // Disable OE (LEDs off) before address change
                    gpio_put(OE, 1);
                    send_row_address(row); 

                    // --- 3. DMA Transfer to PIO ---
                    // Trigger DMA to shove the buffer into the PIO FIFO
                    dma_channel_transfer_from_buffer_now(dma_chan_hub75, scanline_buffer, PANEL_WIDTH);
                    
                    // Wait for the DMA to finish sending all pixels
                    dma_channel_wait_for_finish_blocking(dma_chan_hub75);
                    
                    // Wait for PIO to drain (ensure last pixel is clocked out)
                    // The DMA finishes when it writes the last word to FIFO, 
                    // we need to wait a tiny bit for PIO to shift it out.
                    while(!pio_sm_is_tx_fifo_empty(pio_hub75, sm_hub75)) tight_loop_contents();
                    
                    // Small delay to ensure the last clock pulse finished
                    busy_wait_at_least_cycles(50);

                    // --- 4. Latch & Display ---
                    gpio_put(LAT, 1);
                    busy_wait_at_least_cycles(50); // Hold latch
                    gpio_put(LAT, 0);

                    // BCM Delay
                    uint32_t weight_us = (uint32_t)LSB_TIME_US << (BCM_BITS - 1 - bit_plane);
                    
                    gpio_put(OE, 0); // Enable Output (LEDs ON)
                    sleep_us(weight_us);
                    gpio_put(OE, 1); // Disable Output (LEDs OFF)
                }
            }
        }

        // Check for buffer swap
        if (fbf_swap_request) {
            fbf_rdy = !fbf_rdy;
            fbf_swap_request = false;
        }
    }
}