#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"

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
#define REPEAT_NUM 8

//buffers that store processed audio
static uint8_t audiobuffer[2][BUFFER_SIZE];

//current position in audio file
static int song_pos = 0;
static int sfx_pos = 0;

//current audio track pointers and lengths
static const uint8_t *curr_song = SILENCE_DATA;
static const uint8_t *curr_sfx = SILENCE_DATA;
static int curr_song_len = SILENCE_DATA_LENGTH;
static int curr_sfx_len = SILENCE_DATA_LENGTH;

//audio state index (see audio_track structs below)
static int song_id = 0;
static int sfx_id = 0;
static bool active_buffer = 0; //which buffer is currently being played by DMA

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

//mix audio and fill the specified buffer
static void fill_audio_buffer(int buffer_index) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        //get song sample (loop if needed)
        uint8_t song_sample = curr_song[song_pos];
        song_pos = (song_pos + 1) % curr_song_len;
        
        //get SFX sample if active
        uint8_t sfx_sample = 0;
        if (sfx_id != 0 && sfx_pos < curr_sfx_len) {
            sfx_sample = curr_sfx[sfx_pos];
            sfx_pos++;
            
            //if SFX ended, switch to silence
            if (sfx_pos >= curr_sfx_len) {
                sfx_id = 0;
                curr_sfx = SILENCE_DATA;
                curr_sfx_len = SILENCE_DATA_LENGTH;
                sfx_pos = 0;
            }
        }
        
        //mix samples (average to prevent clipping)
        if (sfx_sample != 0) {
            audiobuffer[buffer_index][i] = (song_sample + sfx_sample) / 2;
        } else {
            audiobuffer[buffer_index][i] = song_sample;
        }
    }
}

static void init_audio_pwm() {
    //configure GPIO for PWM
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    
    //calculate clock divider for desired sample rate
    float clock_div = (150000000.0f / (256.0 * SAMPLE_RATE)) / REPEAT_NUM;
    pwm_set_clkdiv(pwm_gpio_to_slice_num(AUDIO_PIN), clock_div);
    
    //set PWM parameters
    pwm_set_wrap(pwm_gpio_to_slice_num(AUDIO_PIN), 256);
    pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_PIN), pwm_gpio_to_channel(AUDIO_PIN), 0);
    
    //start PWM
    pwm_set_enabled(pwm_gpio_to_slice_num(AUDIO_PIN), true);
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
    
    //claim DMA channel
    dma_chan = dma_claim_unused_channel(true);
    
    //configure DMA for PWM transfer
    dma_config = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, DREQ_PWM_WRAP0 + pwm_gpio_to_slice_num(AUDIO_PIN));
    
    //setup DMA completion interrupt
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_audio_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    
    //configure and start DMA with first buffer
    dma_channel_configure(
        dma_chan,
        &dma_config,
        &pwm_hw->slice[pwm_gpio_to_slice_num(AUDIO_PIN)].cc,
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
        sfx_id = id;
        curr_sfx = sfx[id].data;
        curr_sfx_len = sfx[id].length;
        sfx_pos = 0;
    } else {
        song_id = id;
        curr_song = songs[id].data;
        curr_song_len = songs[id].length;
        song_pos = 0;
    }
}