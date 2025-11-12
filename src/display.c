#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"

// 64x64 RGB LED Matrix - 3mm Pitch - 192mm x 192mm; Product ID: 4732
// https://www.adafruit.com/product/4732
// -----------------------------------  -----------------------------------
// |  1  2  3  4  5  6  7  8        | [R1] [G1] [B1] [R2] [G2] [B2] [A] [B] 
// |  9 10 11 12 13 14 15 16       | [C] [D ] [E ] [CLK][LAT][OE][GND][GND]
// -----------------------------------  -----------------------------------

// assigning GPIOs to data cable ports for adafruit display ---> adjust numbers as needed
#define R1 10 // red data for top half
#define G1 11 // green data for top half
#define B1 12 // blue data for top half
#define R2 13 // red data for bottom half
#define G2 14 // green data for bottom half
#define B2 15 // blue data for bottom half
#define A 16 // row select bit 0
#define B 17 // row select bit 1
#define C 18 // row select bit 2
#define D 19 // row select bit 3
#define E 20 // row select bit 4
#define CLK 21 // clock (shift register)
#define LAT 22 // stores shifted data into output register --> latch
#define OE 23 // active low output enable

// panel dimensions
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANEL_ROWS 32 // # of row addresses (scans 32 times to cover all 64 physical rows by scanning upper half and lower half)

#define GPIO_MASK ((1u<<R1) | (1u<<G1) | (1u<<B1) | (1u<<R2) | (1u<<G2) | (1u<<B2) | (1u<<A) | (1u<<B) | (1u<<C) | (1u<<D) | (1u<<E) | (1u<<CLK) | (1u<<LAT) | (1u<<OE))


// https://github.com/hzeller/rpi-rgb-led-matrix

// framebuffer [row][col][rgb] 0 3d array that stores what to display
uint32_t framebuffer[PANEL_HEIGHT][PANEL_WIDTH][3];

// volatile uint32_t dma_buffer[PANEL_WIDTH * 2] __attribute__((aligned(4)));
uint32_t dma_buffer[PANEL_WIDTH * 2]; // double buffer width
int dma_chan;


// init gpio pins for all display pins
void display_init()
{
    int pins[] = {R1, G1, B1, R2, G2, B2, A, B, C, D, E, CLK, LAT, OE};
    for (int i = 0; i < 14; i++)
    {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_OUT);
        gpio_put(pins[i], 0);
    }
    
    gpio_put(OE, 1); // start with output disabled
    memset(framebuffer, 0, sizeof(framebuffer)); // clear framebuffer

    sleep_ms(10);
}

// selects waht row of hte display to update
// row 0 is 00000, row 1 is 00001...row 64 is 11111, LSB so EDCBA
void send_row(uint8_t row) 
{
    gpio_put(A, row & 1);
    gpio_put(B, (row >> 1) & 1);
    gpio_put(C, (row >> 2) & 1);
    gpio_put(D, (row >> 3) & 1);
    gpio_put(E, (row >> 4) & 1);

}

// prepare DMA buffer by adding all the GPIO states needed to make one complete row
void prepare_row_data(uint8_t row, uint8_t bit_plane) 
{
    uint32_t base_state = sio_hw->gpio_out & ~GPIO_MASK; // preserves non display pins, 0's where display pins are

    base_state |= (1u << OE); // makes OE stay high (disabled) while shifting


    // converts row number to binary and adds to base state
    uint32_t row_bits = 0;
    row_bits |= (row & 1) ? (1u << A) : 0;
    row_bits |= ((row >> 1) & 1) ? (1u << B) : 0;
    row_bits |= ((row >> 2) & 1) ? (1u << C) : 0;
    row_bits |= ((row >> 3) & 1) ? (1u << D) : 0;
    row_bits |= ((row >> 4) & 1) ? (1u << E) : 0;
    base_state |= row_bits;

    for (int col = 0; col < PANEL_WIDTH; col++) // building pixel data for each column
    {
        uint32_t pixel_bits = 0;
        
        // upper half (rows 0-31)
        uint8_t r1 = framebuffer[row][col][0];
        uint8_t g1 = framebuffer[row][col][1];
        uint8_t b1 = framebuffer[row][col][2];
        
        // lower half (rows 32-63)
        uint8_t r2 = framebuffer[row + 32][col][0];
        uint8_t g2 = framebuffer[row + 32][col][1];
        uint8_t b2 = framebuffer[row + 32][col][2];
        
        // implementing bit plane modulation (brightness manipulation)
        // if bit plane = 0, threshold = 128 (MSB)
        // if bit plane = 1, threshold = 64
        // if bit plane = 2, threshold = 32 (LSB)
        // i.e., if red = 200 (binary is 11001000)
            // bit plane 0: 200 > 128 --> true so LED on for 50 us
            // bit plane 1: 200 > 64 --> true so LED on for 25 us
            // bit plane 2: 200 > 32 --> true so LED on for 12.5 us
        if (r1 > (128 >> bit_plane)) pixel_bits |= (1u << R1);
        if (g1 > (128 >> bit_plane)) pixel_bits |= (1u << G1);
        if (b1 > (128 >> bit_plane)) pixel_bits |= (1u << B1);
        if (r2 > (128 >> bit_plane)) pixel_bits |= (1u << R2);
        if (g2 > (128 >> bit_plane)) pixel_bits |= (1u << G2);
        if (b2 > (128 >> bit_plane)) pixel_bits |= (1u << B2);
        
        // creating clock rising edges; each pixel needs clock pulse to shift into the matrix's registers
        dma_buffer[col * 2] = base_state | pixel_bits;// stores CLK=0 state
        dma_buffer[col * 2 + 1] = base_state | pixel_bits | (1u << CLK); // stpres CLK=1 state (same but with clock high)
    }
}

void dma_init()
{
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);

    channel_config_set_dreq(&c, DREQ_FORCE); // goes as fast as possible

    // write address (GPIO_OUT), read address dma pattern buffer, 128 transfers, don't start yet)
    dma_channel_configure(dma_chan, &c, &sio_hw->gpio_out, dma_buffer, PANEL_WIDTH * 2, false);
}


// use DMA tos end one complete row of data to the display
void send_row_dma(uint8_t row, uint8_t bit_plane) 
{
    // disables output while updating
    gpio_put(OE, 1);
    
    // prepares DMA buffer with pixel data
    prepare_row_data(row, bit_plane);
    
    // configures and start DMA transfer
    dma_channel_set_read_addr(dma_chan, dma_buffer, false);
    dma_channel_set_trans_count(dma_chan, PANEL_WIDTH * 2, true);  // transfers 128 values (64 pixels * 2 clock edges)
    
    // waits for DMA to finish
    dma_channel_wait_for_finish_blocking(dma_chan);
    
    // latches the data
    gpio_put(LAT, 1);
    sleep_us(1);
    gpio_put(LAT, 0);
    
    //display for appropriate duration
    gpio_put(OE, 0);
    sleep_us(50 >> bit_plane);    // bit plane timing: more significant bits stay on longer, brightness control
    gpio_put(OE, 1); // disables output
    }

// scans through all rows and bit plans to show full image
void display_refresh()
{
    // Scan all rows with bit-plane modulation for brightness
    for (uint8_t bit_plane = 0; bit_plane < 3; bit_plane++) // 3 bit planes, 2^3 brightness levels per color channel
    {
        for (uint8_t row = 0; row < PANEL_ROWS; row++)  // 64 rows covered
        {
            send_row_dma(row, bit_plane);
        }
    }
}

// sets a pixel in the framebuffer
// takes (x,y) coordinates with rgb values and stores into framebuffer array
// basically, allows each entire image to be built first before getting displayed
void display_set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= PANEL_WIDTH || y >= PANEL_HEIGHT) 
    {
        return;
    }

    framebuffer[y][x][0] = r;
    framebuffer[y][x][1] = g;
    framebuffer[y][x][2] = b;
}


//////////////////////////////// helper functions
// clears the display
void display_clear()
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

// fills entire screen with solid color
void display_fill(uint8_t r, uint8_t g, uint8_t b)
{
    for (int y = 0; y < PANEL_HEIGHT; y++) 
    {
        for (int x = 0; x < PANEL_WIDTH; x++) 
        {
            framebuffer[y][x][0] = r;
            framebuffer[y][x][1] = g;
            framebuffer[y][x][2] = b;
        }
    }
}

// draw a filled rectangle, maybe used for tetris blocks
void display_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t g, uint8_t b)
{
    for (uint8_t dy = 0; dy < h; dy++) 
    {
        for (uint8_t dx = 0; dx < w; dx++) 
        {
            display_set_pixel(x + dx, y + dy, r, g, b);
        }
    }
}

// test pattern
void display_test_pattern()
{
    // quadrant test
    display_draw_rect(0, 0, 32, 32, 255, 0, 0); // red top left
    display_draw_rect(32, 0, 32, 32, 0, 255, 0); // green top right
    display_draw_rect(0, 32, 32, 32, 0, 0, 255); // blue bottom left
    display_draw_rect(32, 32, 32, 32, 255, 255, 0); // yellow bottom right
    
    // white border
    for (int i = 0; i < PANEL_WIDTH; i++) 
    {
        display_set_pixel(i, 0, 255, 255, 255);
        display_set_pixel(i, PANEL_HEIGHT-1, 255, 255, 255);
    }
    for (int i = 0; i < PANEL_HEIGHT; i++) 
    {
        display_set_pixel(0, i, 255, 255, 255);
        display_set_pixel(PANEL_WIDTH-1, i, 255, 255, 255);
    }
}



int main()
{
    stdio_init_all();
    
    display_init();
    dma_init();
    
    // test pattern
    display_test_pattern();
    
    // continuously refresh display
    while (1) 
    {
        display_refresh();
    }
    
    return 0;

}