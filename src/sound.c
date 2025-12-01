#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"

#define FAST_MODE true

#if !FAST_MODE
//title song + 3 game songs + highscore song (can change to diff ending song if desired)
#include "songB.h"
#include "songC.h"
#include "tetoris.h"
//#include "highscore44.h" //can choose between ending song, i wanna check quality
#include "end.h"
#include "title.h"
#endif

#include "songA.h"
#include "silence.h"
#include "clear.h"   //clear SFX, clear 4 lines SFX and game over SFX
#include "clear4.h"
#include "gameoverSFX.h"

#define AUDIO_PIN 36
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 256
#define WRAP 255

//buffers that store processed audio
static uint32_t audiobuffer[2][BUFFER_SIZE];
//which buffer is currently being played by DMA
static bool active_buffer = 0;

//current position in audio file
static int song_pos = 0;
static int sfx_pos = 0;

//current audio track pointers and lengths
static const uint8_t *curr_song = SILENCE_DATA;
static const uint8_t *curr_sfx = SILENCE_DATA;
static int curr_song_len = SILENCE_DATA_LENGTH;
static int curr_sfx_len = SILENCE_DATA_LENGTH;

//DMA channel
static int dma_chan;
static dma_channel_config dma_config;

typedef struct {
    const uint8_t *data;
    int length;
} audio_track;

//0: none, 1: songA, 2: songB, 3: songC, 4: tetoris, 5: end, 6: title
static const audio_track songs[] = {
    {SILENCE_DATA, SILENCE_DATA_LENGTH},
    {SONGA_DATA, SONGA_DATA_LENGTH},
    #if !FAST_MODE
    {SONGB_DATA, SONGB_DATA_LENGTH},
    {SONGC_DATA, SONGC_DATA_LENGTH},
    {TETO_DATA, TETO_DATA_LENGTH},
    {END_DATA, END_DATA_LENGTH},
    {TITLE_DATA, TITLE_DATA_LENGTH}
    #endif
};

//0: none, 1: clear, 2: 4-line clear, 3: gameover
static const audio_track sfx[] = {
    {SILENCE_DATA, SILENCE_DATA_LENGTH},
    {CLEAR_DATA, CLEAR_DATA_LENGTH},
    {CLEAR4_DATA, CLEAR4_DATA_LENGTH},
    {GAMEOVER_DATA, GAMEOVER_DATA_LENGTH}
};

//mix audio and fill the buffer. we produce packed 32-bit words:
//low 16 bits -> channel A compare, high 16 bits -> channel B compare
static void fill_audio_buffer(int buffer_index) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        //get song sample (loop if needed)
        uint8_t song_sample = curr_song[song_pos];
        song_pos = (song_pos + 1) % curr_song_len;

        //get SFX sample if active
        uint8_t sfx_sample = 0;
        if (curr_sfx != SILENCE_DATA && sfx_pos < curr_sfx_len) {
            sfx_sample = curr_sfx[sfx_pos++];

            //if SFX ended, switch to silence
            if (sfx_pos >= curr_sfx_len) {
                curr_sfx = SILENCE_DATA;
                curr_sfx_len = SILENCE_DATA_LENGTH;
                sfx_pos = 0;
            }
        }

        //mix samples (average to prevent clipping)
        uint8_t mixed;
        if (sfx_sample != 0) mixed = (song_sample + sfx_sample) / 2;
        else mixed = song_sample;

        #if WRAP != 255
        mixed = (uint16_t)mixed * WRAP / 255;
        #endif

        //pack into 32-bit: both halves equal so both channels follow sample
        uint32_t word = ((uint32_t)mixed << 16) | (uint32_t)mixed;
        audiobuffer[buffer_index][i] = word;
    }
}

static void init_audio_pwm() {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    int slice_num = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_set_wrap(slice_num, WRAP);

    //calculate clock divider for desired sample rate
    uint32_t clk_sys_hz = clock_get_hz(clk_sys);
    float clkdiv = (float)clk_sys_hz / ((float)SAMPLE_RATE * (WRAP + 1.0f));
    pwm_set_clkdiv(slice_num, clkdiv);

    //zero initial levels
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(AUDIO_PIN), 0);
    pwm_set_enabled(slice_num, true);
}

//DMA interrupt handler
//fills the buffer that just finished playing
static void dma_audio_handler() {
    //clear interrupt
    dma_channel_acknowledge_irq0(dma_chan);

    //switch to the other buffer
    active_buffer = !active_buffer;

    dma_channel_set_read_addr(dma_chan, audiobuffer[active_buffer], true);
    
    //fill the buffer that just became available
    fill_audio_buffer(!active_buffer);
}

static void init_audio_dma() {
    //fill both buffers with initial audio
    fill_audio_buffer(0);
    fill_audio_buffer(1);

    int slice_num = pwm_gpio_to_slice_num(AUDIO_PIN);
    
    //claim DMA channel
    dma_chan = dma_claim_unused_channel(true);
    
    //configure DMA for PWM transfer
    dma_config = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, DREQ_PWM_WRAP0 + slice_num);
    
    //setup DMA completion interrupt
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_audio_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    
    //configure and start DMA with first buffer
    dma_channel_configure(
        dma_chan,
        &dma_config,
        &pwm_hw->slice[slice_num].cc,
        audiobuffer[0],
        BUFFER_SIZE,
        true
    );
    
    active_buffer = 0;
}

void init_audio() {
    init_audio_pwm();
    init_audio_dma();
}

void play_audio(int id, int is_sfx) {
    //reset position and set new audio
    if (is_sfx) {
        curr_sfx = sfx[id].data;
        curr_sfx_len = sfx[id].length;
        sfx_pos = 0;
    } else {
        curr_song = songs[id].data;
        curr_song_len = songs[id].length;
        song_pos = 0;
    }
}