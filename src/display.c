#include "pico/stdlib.h"

// -----------------------------------  -----------------------------------
// |  1  2  3  4  5  6  7  8  9       | [R1] [G1] [B1] [R2] [G2] [B2] [A] [B] [C]
// | 10 11 12 13 14 15 16 17 18       | [D ] [E ] [CLK][LAT][OE][GND][GND][GND][GND]
// -----------------------------------  -----------------------------------

// assigning GPIOs to data cable ports for adafruit display ---> adjust numbers as needed
#define R1 0 // red data for top half
#define G1 1 // green data for top half
#define B1 2 // blue data for top half
#define R2 3 // red data for bottom half
#define G2 4 // green data for bottom half
#define B2 5 // blue data for bottom half
#define A 6 // row select bit 0
#define B 7 // row select bit 1
#define C 8 // row select bit 2
#define D 9 // row select bit 3
#define E 10 // row select bit 4
#define CLK 11 // clock (shift register)
#define LAT 12 // stores shifted data into output register --> latch
#define OE 13 // active low output enable

// https://github.com/hzeller/rpi-rgb-led-matrix

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

// shifts one "pixel" of data into the display for the current row
void shift_pixel(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) 
{
    // set the color data bits for top & bottom halves
    gpio_put(R1, r1);
    gpio_put(G1, g1);
    gpio_put(B1, b1);
    gpio_put(R2, r2);
    gpio_put(G2, g2);
    gpio_put(B2, b2);

    // pulses clock to shift this data into the panel's internal shift registers
    gpio_put(CLK, 1);
    sleep_us(1);
    gpio_put(CLK, 0);
}

// latch/store the shifted row data and display it briefyl
void latch_display(void) {
    // toggles LAT high then low — this moves the shifted bits into the active drivers
    gpio_put(LAT, 1);
    sleep_us(1);
    gpio_put(LAT, 0);

    // enables LED output --> OE = 0 since active low
    gpio_put(OE, 0);
    sleep_ms(2);
    gpio_put(OE, 1); // disables output, prevents ghosting before next row
}

int main()
{
    stdio_init_all();

    display_init();

    for(;;)
    {
        for (int row = 0; row < 32; row++) 
        {
            // selects which row pair to display (matrix split into 2 halves --> top 32 rows, lower 32 rows)
            send_row(row);

            // for each of the 64 columns, send one pixel's worth of color data
            for (int col = 0; col < 64; col++) 
            {
                // top half gets red, bottom half gets green
                // top pixel = red (1, 0, 0)
                // bottom pixel = green (0, 1, 0) 
                shift_pixel(1, 0, 0, 0, 1, 0);
            }

            // latch/store shifted data and briefly display the row
            latch_display();
        }
        
    }

}