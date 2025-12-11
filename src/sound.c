#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "sound.h"

#define FAST_MODE true

#if !FAST_MODE
#include "title.h"
#include "themeB.h"
#include "themeC.h"
#endif

#include "silence.h"
#include "themeA.h"

#include "clear.h"
#include "clear4.h"
#include "game_over.h"
#include "game_start.h"
#include "move.h"
#include "rotate.h"
#include "piece_lock.h"
#include "select_option.h"
#include "switch_option.h"
#include "stage_clear.h"
#include "touch_surface.h"
#include "garbage.h"
#include "pause.h"
#include "soft_drop.h"
#include "game_win.h"

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

int song_volume = SONG_VOLUME_DEFAULT;
int sfx_volume = SFX_VOLUME_DEFAULT;
bool song_paused = false;
int song_choice = THEMEA_SONG;

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

static const audio_track songs[] = {
    {SILENCE_DATA, SILENCE_DATA_LENGTH},
    {THEMEA_DATA, THEMEA_DATA_LENGTH},
    #if !FAST_MODE
    {THEMEB_DATA, THEMEB_DATA_LENGTH},
    {THEMEC_DATA, THEMEC_DATA_LENGTH},
    {TITLE_DATA, TITLE_DATA_LENGTH},
    #endif
};

static const audio_track sfx[] = {
    {SILENCE_DATA, SILENCE_DATA_LENGTH},
    {CLEAR_DATA, CLEAR_DATA_LENGTH},
    {CLEAR4_DATA, CLEAR4_DATA_LENGTH},
    {GAME_OVER_DATA, GAME_OVER_DATA_LENGTH},
    {GAME_START_DATA, GAME_START_DATA_LENGTH},
    {MOVE_DATA, MOVE_DATA_LENGTH},
    {ROTATE_DATA, ROTATE_DATA_LENGTH},
    {PIECE_LOCK_DATA, PIECE_LOCK_DATA_LENGTH},
    {SELECT_OPTION_DATA, SELECT_OPTION_DATA_LENGTH},
    {SWITCH_OPTION_DATA, SWITCH_OPTION_DATA_LENGTH},
    {STAGE_CLEAR_DATA, STAGE_CLEAR_DATA_LENGTH},
    {TOUCH_SURFACE_DATA, TOUCH_SURFACE_DATA_LENGTH},
    {GARBAGE_DATA, GARBAGE_DATA_LENGTH},
    {PAUSE_DATA, PAUSE_DATA_LENGTH},
    {SOFT_DROP_DATA, SOFT_DROP_DATA_LENGTH},
    {GAME_WIN_DATA, GAME_WIN_DATA_LENGTH},
};

//mix audio and fill the buffer. we produce packed 32-bit words:
//low 16 bits -> channel A compare, high 16 bits -> channel B compare
static void fill_audio_buffer(int buffer_index) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        uint8_t song_sample = 128;
        if (!song_paused) {
            //get song sample (loop if needed)
            song_sample = curr_song[song_pos];
            song_pos = (song_pos + 1) % curr_song_len;
        }

        //get sfx sample if active
        uint8_t sfx_sample = 128;  //default silence
        if (curr_sfx != SILENCE_DATA && sfx_pos < curr_sfx_len) {
            sfx_sample = curr_sfx[sfx_pos++];

            //if SFX ended, switch to silence
            if (sfx_pos >= curr_sfx_len) {
                curr_sfx = SILENCE_DATA;
                curr_sfx_len = SILENCE_DATA_LENGTH;
                sfx_pos = 0;
            }
        }

        //convert to signed -128..127
        int song_s = (int)song_sample - 128;
        int sfx_s  = (int)sfx_sample  - 128;

        //apply volumes and mix
        int mixed_s = (song_s * song_volume + sfx_s * sfx_volume) / (100 * 2);

        //clamp
        if (mixed_s < -128) mixed_s = -128;
        if (mixed_s > 127)  mixed_s = 127;

        //back to unsigned
        uint8_t mixed = (uint8_t)(mixed_s + 128);

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
        if (FAST_MODE && id > 1) id = SILENCE_SONG;
        
        curr_song = songs[id].data;
        curr_song_len = songs[id].length;
        song_pos = 0;
    }
}