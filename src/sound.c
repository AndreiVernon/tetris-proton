#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/sync.h"

#include "songA44.h"
#include "clear.h"
//temp w/o DMA for buffer for now
#define AUDIO_PIN 36
int posA=0;//position in wav file, if implement dma will alter this process..
int posB=0;
int dma_chan;
int line_clear=0;

void pwm_audio_handler(){
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));
    //wanna implement so that it loops the file
    //have shifts since is interrupted multiple times per
    if(line_clear==0){
        if(posA<(SONGA_DATA_LENGTH<<3)-1){
            int A=SONGA_DATA[posA>>3];
            pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_PIN),pwm_gpio_to_channel(AUDIO_PIN),A);
            posA++;
        }
        else{//if has reached end, loop to beginning
            posA=0;
        }
    }
    else{
        if(posA<(SONGA_DATA_LENGTH<<3)-1 && posB<(CLEAR_DATA_LENGTH<<3)-1){
            int A=SONGA_DATA[posA>>3];
            int B=CLEAR_DATA[posB>>3];
            int C=(A+B)/2.0;
            pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_PIN),pwm_gpio_to_channel(AUDIO_PIN),C);
            posA++;
            posB++;
        }
        else{
            if(!(posA<(SONGA_DATA_LENGTH<<3)-1)){//if has reached end, loop to beginning
                posA=0;
            }
            if(!(posB<(CLEAR_DATA_LENGTH<<3)-1)){
                posB=0;
                line_clear=0;
            }
        }
    }
}

void init_pwm_audio() {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    
    //want intterupt freq to be higher than audio freq
    pwm_set_clkdiv(pwm_gpio_to_slice_num(AUDIO_PIN),(150000000.0f/(256.0*44100.0f))/8.0f); //rn have output at about 88khz so it can work with 44,22,11,
    pwm_hw->slice[pwm_gpio_to_slice_num(AUDIO_PIN)].top=255;
    pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_PIN),pwm_gpio_to_channel(AUDIO_PIN),0);

    pwm_set_irq_enabled(pwm_gpio_to_slice_num(AUDIO_PIN),1);
    irq_set_exclusive_handler(PWM_IRQ_WRAP_0,pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP_0,1);
    pwm_hw->slice[pwm_gpio_to_slice_num(36)].csr=1<<0;
}

// int main(){//temp to check if runs
//     stdio_init_all();

//     init_pwm_audio();

//     gpio_init(21);
//     gpio_set_dir(21,0);

//     while(true){
//         if(gpio_get(21)!=0){
//             line_clear=1;
//         }

//     }
// }




































//OLD

// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/dma.h"
// #include "hardware/irq.h"
// #include "hardware/pwm.h"

// #define AUDIO_PIN 36

// #include "title44.h"
// #include "songA44.h"

// // The fixed location the sample DMA channel writes to and the PWM DMA channel
// // reads from
// uint32_t single_sample = 0;
// uint32_t* single_sample_ptr = &single_sample;

// //for changing songs idea is global variable will represent like menuselect, game start, gameover
// //and dep on those i will run a diff song if to correspond to that song
// //0 for title, 1 for game song(maybe implement random/choosing song later), 2 for game end song
// int songid=0;

// int pwm_dma_chan, trigger_dma_chan, sample_dma_chan;

// #define REPETITION_RATE 4

// void dma_irh() {
//     if(songid==0){
//         dma_hw->ch[sample_dma_chan].al1_read_addr=TITLE_DATA;
//     }
//     else if(songid==1){
//         dma_hw->ch[sample_dma_chan].al1_read_addr=SONGA_DATA;
//     }
    
//     dma_hw->ch[trigger_dma_chan].al3_read_addr_trig=&single_sample_ptr;
//     dma_hw->ints0 = (1u << trigger_dma_chan);
// }

// //DMA is being annoying when trying to send 8 bit, seems like theres some issue when not read and writing 32 bit
// //so use multiple channels
// //sample reads 1 val from wavdata at time and saves it, is chained so once samples pwm starts
// //pwm reads that value 4 time, which mean val is now 32 bit and reptetion makes sound good
// //trigger is needed to start sample 
// //aka trigger->pwm reads repeat->sample reads->trigger again etc etc
// void init_pwm_dma(){
//     gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
//     uint audio_pin_slice=pwm_gpio_to_slice_num(AUDIO_PIN);
//     uint channel=pwm_gpio_to_channel(AUDIO_PIN);

//     pwm_set_wrap(audio_pin_slice, 255);
//     pwm_set_clkdiv(audio_pin_slice, (150000000.0f/(256.0f*44100.0f))/REPETITION_RATE);//sysclk/(8bits * samplerate)
//     pwm_set_chan_level(audio_pin_slice, channel, 0);
//     pwm_set_enabled(audio_pin_slice, 1);

//     pwm_dma_chan = dma_claim_unused_channel(true);
//     trigger_dma_chan = dma_claim_unused_channel(true);
//     sample_dma_chan = dma_claim_unused_channel(true);

//     dma_hw->ch[pwm_dma_chan].read_addr=(u_int32_t)&single_sample;
//     dma_hw->ch[pwm_dma_chan].write_addr=(u_int32_t)&pwm_hw->slice[audio_pin_slice].cc;
//     dma_hw->ch[pwm_dma_chan].transfer_count=REPETITION_RATE;
//     uint32_t pwmtrig=0;
//     pwmtrig|=(2<<2 | (DREQ_PWM_WRAP0+audio_pin_slice)<<17 | sample_dma_chan<<13 | 1<<0);
//     dma_hw->ch[pwm_dma_chan].ctrl_trig=pwmtrig;

//     dma_hw->ch[trigger_dma_chan].read_addr=(u_int32_t)&single_sample_ptr;//since want pwmdma to read from singlesample
//     dma_hw->ch[trigger_dma_chan].write_addr=(u_int32_t)&dma_hw->ch[pwm_dma_chan].al3_read_addr_trig;
//     dma_hw->ch[trigger_dma_chan].transfer_count=REPETITION_RATE*TITLE_DATA_LENGTH;
//     uint32_t trig=0;
//     trig|=(2<<2 | (DREQ_PWM_WRAP0+audio_pin_slice)<<17 | 1<<0);
//     dma_hw->ch[trigger_dma_chan].ctrl_trig=trig;
    

//     //rather than looping with ring, size might be not nice number so instead use interrupts to restart
//     //trigger when trigger done which is full loop
//     dma_channel_set_irq0_enabled(trigger_dma_chan, true);
//     irq_set_exclusive_handler(DMA_IRQ_0, dma_irh);
//     irq_set_enabled(DMA_IRQ_0, true);

//     dma_hw->ch[sample_dma_chan].read_addr=(u_int32_t)TITLE_DATA;
//     dma_hw->ch[sample_dma_chan].write_addr=(u_int32_t)&single_sample;
//     dma_hw->ch[sample_dma_chan].transfer_count=1;
//     uint32_t samptrig=0;
//     samptrig|=(1<<4 | 0<<2 | (DREQ_FORCE)<<17 | 0<<13 | 1<<0);
//     dma_hw->ch[sample_dma_chan].ctrl_trig=samptrig;

// }
// void play_audio(){
//     dma_channel_abort(pwm_dma_chan);
//     dma_channel_abort(trigger_dma_chan);
//     dma_channel_abort(sample_dma_chan);

//     if(songid==0){
//         dma_hw->ch[sample_dma_chan].read_addr=(u_int32_t)TITLE_DATA;
//         dma_hw->ch[trigger_dma_chan].transfer_count=REPETITION_RATE*TITLE_DATA_LENGTH;
//     }
//     else if(songid==1){
//         dma_hw->ch[sample_dma_chan].read_addr=(u_int32_t)SONGA_DATA;
//         dma_hw->ch[trigger_dma_chan].transfer_count=REPETITION_RATE*SONGA_DATA_LENGTH;
//     }

//     dma_channel_start(trigger_dma_chan);
// }


// int main(void) {
//     stdio_init_all();
//     init_pwm_dma();

//     gpio_init(21);
//     gpio_set_dir(21,0);
//     //for now have gpio21 button val be like game start
    
    
//     dma_channel_start(trigger_dma_chan);

//     while(1) {
//         if(gpio_get(21)!=0){
//             songid=1;
//             play_audio();
//         }
        
//     }
// }
