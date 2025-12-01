#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"

//title song + 3 game songs + highscore song (can change to diff ending song if desired)
#include "title.h"
// #include "songA.h"
// #include "songB.h"
// #include "songC.h"
// #include "tetoris.h"
//#include "highscore44.h" //can choose between ending song, i wanna check quality
// #include "end.h"
#include "clear.h"   //clear SFX, clear 4 lines SFX and game over SFX
#include "clear4.h"
#include "gameoverSFX.h"

#include "silence.h"

const uint8_t *currsong = TITLE_DATA;
const uint8_t *currsfx = SILENCE_DATA;  //just empty so it doesn't read bad just in case

int currsongid = 0;  //have these be to see if song was changed
int currsfxid = 0;

#define AUDIO_PIN 36
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 256  //keep as basic num for now maybe forever
#define REPEAT_NUM 8

uint8_t songbuffer[BUFFER_SIZE];  //two diff buffers
uint8_t sfxbuffer[BUFFER_SIZE];

int songpos = 0;       //position in wav file for song & SFX (should be only two things overlapping)
int sfxpos = 0;
int songbufferpos = 0; //positions in buffer
int sfxbufferpos = 0;

int songid = 6; //is 1/2/3 for A/B/C 4 tetoris and 5 gameend (could add almost losing)
int sfxid = 0;  //0 if none, 1: clear, 2: 4 line clear, 3: gameover

int songlength = TITLE_DATA_LENGTH;  //start with title initially
int song_dma_chan;
int sfxlength = 0;  //and no sfx initially
int sfx_dma_chan;

int writingsong = 1;  //vars to check whether to pwm or dma
int writingsfx = 1;

//setup dma for song buffer
void init_song_dma() {
    song_dma_chan = dma_claim_unused_channel(true);

    // songlength = SONGA_DATA_LENGTH;
    dma_hw->ch[song_dma_chan].read_addr = (uint32_t) currsong;
    dma_hw->ch[song_dma_chan].write_addr = (uint32_t) songbuffer;
    dma_hw->ch[song_dma_chan].transfer_count = BUFFER_SIZE;

    uint32_t song_trig = 0;
    song_trig = (1 << 0) | (0 << 2) | (1 << 4) | (1 << 6) | (DMA_CH10_CTRL_TRIG_TREQ_SEL_VALUE_PERMANENT << 17);

    dma_hw->ch[song_dma_chan].ctrl_trig = song_trig;
}

//setup dma for sfx buffer
void init_sfx_dma() {
    sfx_dma_chan = dma_claim_unused_channel(true);

    dma_hw->ch[sfx_dma_chan].read_addr = (uint32_t) currsfx;
    dma_hw->ch[sfx_dma_chan].write_addr = (uint32_t) sfxbuffer;
    dma_hw->ch[sfx_dma_chan].transfer_count = BUFFER_SIZE;

    uint32_t sfx_trig = 0;
    sfx_trig = (1 << 0) | (0 << 2) | (1 << 4) | (1 << 6) | (DMA_CH10_CTRL_TRIG_TREQ_SEL_VALUE_PERMANENT << 17);

    dma_hw->ch[sfx_dma_chan].ctrl_trig = sfx_trig;
}

//handles writing to pwm
void pwm_audio_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));

    int A;
    if (sfxid != 0) {  //if sfx is curr playing
        A = (songbuffer[songbufferpos >> 3] + sfxbuffer[sfxbufferpos >> 3]) / 2;
    } else {
        A = songbuffer[songbufferpos >> 3];  //else just song
    }

    //if both not done/no sfx song not done
    if (songbufferpos < ((BUFFER_SIZE << 3) - 1) && (sfxid != 0 ? sfxbufferpos < ((BUFFER_SIZE << 3) - 1) : 1)) {

        writingsong = 1;
        if (sfxid != 0) writingsfx = 1;

        pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_PIN), pwm_gpio_to_channel(AUDIO_PIN), A);

        songbufferpos++;
        if (sfxid != 0) sfxbufferpos++;
    }

    //if song has reached end, loop to beginning
    if (!(songbufferpos < ((BUFFER_SIZE << 3) - 1))) {
        songbufferpos = 0;
        writingsong = 0;
    }

    //same for sfx buffer
    if (!(sfxbufferpos < ((BUFFER_SIZE << 3) - 1))) {
        sfxbufferpos = 0;
        writingsfx = 0;
    }

    //but if sfx wav file at end then stop and dont repeat
    if (sfxpos >= sfxlength) {
        sfxid = 0;
        currsfxid = 0;
    }
}

//setup pwm
void init_pwm_audio() {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    //want intterupt freq to be higher than audio freq
    pwm_set_clkdiv(pwm_gpio_to_slice_num(AUDIO_PIN), (150000000.0f / (256.0 * SAMPLE_RATE)) / REPEAT_NUM);

    pwm_hw->slice[pwm_gpio_to_slice_num(AUDIO_PIN)].top = 256;
    pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_PIN), pwm_gpio_to_channel(AUDIO_PIN), 0);

    pwm_set_irq_enabled(pwm_gpio_to_slice_num(AUDIO_PIN), 1);
    irq_set_exclusive_handler(PWM_IRQ_WRAP_0, pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP_0, 1);

    pwm_hw->slice[pwm_gpio_to_slice_num(36)].csr = (1 << 0);
}


void song_select() {
    if (songid == 0) {
        currsong = TITLE_DATA;
        songlength = TITLE_DATA_LENGTH;
        currsongid = 0;
    } else if (songid == 1) {
    //     currsong = SONGA_DATA;
    //     songlength = SONGA_DATA_LENGTH;
    //     currsongid = 1;
    // } else if (songid == 2) {
    //     currsong = SONGB_DATA;
    //     songlength = SONGB_DATA_LENGTH;
    //     currsongid = 2;
    // } else if (songid == 3) {
    //     currsong = SONGC_DATA;
    //     songlength = SONGC_DATA_LENGTH;
    //     currsongid = 3;
    // } else if (songid == 4) {
    //     currsong = TETO_DATA;
    //     songlength = TETO_DATA_LENGTH;
    //     currsongid = 4;
    // } else if (songid == 5) {
    //     currsong = END_DATA;
    //     songlength = END_DATA_LENGTH;
    //     currsongid = 5;
    }

    songpos = 0;  //for changing song start at pos 0
}


void sfx_select() {
    if (sfxid == 0) {
        sfxlength = 0;
        currsfxid = 0;
    } else if (sfxid == 1) {
        currsfx = CLEAR_DATA;
        sfxlength = CLEAR_DATA_LENGTH;
        currsfxid = 1;
    } else if (sfxid == 2) {
        currsfx = CLEAR4_DATA;
        sfxlength = CLEAR4_DATA_LENGTH;
        currsfxid = 2;
    } else if (sfxid == 3) {
        currsfx = GAMEOVER_DATA;
        sfxlength = GAMEOVER_DATA_LENGTH;
        currsfxid = 3;
    }

    sfxpos = 0;
}

void play_audio() {
    //song selection
    if (songid != currsongid) {  //if changing songs (game start/end/reset)
        song_select();
    }

    if (sfxid != currsfxid) {
        sfx_select();
    }

    //if not busy aka done copying then copy from next addr
    if (writingsong == 0 && !(dma_hw->ch[song_dma_chan].ctrl_trig & DMA_CH0_CTRL_TRIG_BUSY_BITS)) {

        songpos += BUFFER_SIZE;

        if (songpos >= songlength) {
            songpos = 0;
        }

        dma_hw->ch[song_dma_chan].read_addr = (uint32_t) (currsong + songpos);
        dma_hw->ch[song_dma_chan].write_addr = (uint32_t) songbuffer;
        dma_hw->ch[song_dma_chan].transfer_count = BUFFER_SIZE;
        dma_hw->ch[song_dma_chan].ctrl_trig |= 1;
    }

    //same for sfx
    if (writingsfx == 0 && !(dma_hw->ch[sfx_dma_chan].ctrl_trig & DMA_CH0_CTRL_TRIG_BUSY_BITS)) {

        dma_hw->ch[sfx_dma_chan].read_addr = (uint32_t) (currsfx + sfxpos);
        dma_hw->ch[sfx_dma_chan].write_addr = (uint32_t) sfxbuffer;
        dma_hw->ch[sfx_dma_chan].transfer_count = BUFFER_SIZE;
        dma_hw->ch[sfx_dma_chan].ctrl_trig |= 1;

        sfxpos += BUFFER_SIZE;

        if (sfxpos >= sfxlength) {  //adding check
            sfxpos = 0;
            sfxid = 0;
        }
    }
}

void init_audio() {
    init_pwm_audio();
    init_song_dma();
    init_sfx_dma();
    play_audio();
}

//just to show how i was testing it
// int main() {//temp to check if runs
//     stdio_init_all();

//     init_pwm_audio();

//     gpio_init(21);
//     gpio_set_dir(21,0);
//     gpio_init(26);
//     gpio_set_dir(26,0);
//     // int userinput=0;
//     while(true){
//         play_audio();
//         if(gpio_get(21)!=0){//test cycling through songs
//             songid++;
//             if(songid==6){
//                 songid=0;
//             }
//             sleep_ms(200);
//         }
//         if(gpio_get(26)!=0){//test cycling through songs
//             sfxid=3;
//             // sleep_ms(100);
//         }
//     }
// }