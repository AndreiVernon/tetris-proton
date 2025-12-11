#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__

#include "pico/stdlib.h"

typedef enum {
    mp_msg_none        = 0x00,
	mp_msg_ping        = 0x01,
	mp_msg_pong        = 0x02,
	mp_msg_send_lines  = 0x03,
	mp_msg_game_over   = 0x04,
	mp_msg_pause       = 0x05,
	mp_msg_level_opt   = 0x06,
	mp_msg_grav_opt    = 0x07,
	mp_msg_grav_opt_hi = 0x08,
} mp_msg_t;

extern volatile bool received_ping;
extern volatile bool mp_sync_ready;
extern volatile bool mp_sync_awaiting;
extern volatile int mp_pause_received;
extern volatile bool grav_opt_received;
extern volatile uint8_t grav_opt_temp;

void mp_uart_init();
bool mp_send_msg(mp_msg_t msg);
bool mp_send_msg_packed(mp_msg_t msg, uint8_t arg);
void mp_drain_rx();

//try handshake by sending ping and waiting for pong
bool mp_handshake_blocking(uint64_t timeout_us);


#endif