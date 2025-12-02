#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__

#include "pico/stdlib.h"

typedef enum {
	mp_msg_none        = 0x00,
	mp_msg_ping        = 0x01,
	mp_msg_pong        = 0x02,
	mp_msg_send_lines  = 0x03, //request to send N garbage lines; optional extra arg byte may follow
	mp_msg_game_over   = 0x04,
	mp_msg_ack         = 0x05,
	mp_msg_sync        = 0x06, //used during init if desired
	mp_msg_custom_base = 0x10
} mp_msg_t;

//init uart hardware
void multiplayer_uart_init(uint baudrate);
//send a single-message code
bool multiplayer_send_msg(mp_msg_t msg);
//send a message with an extra 1-byte argument
bool multiplayer_send_msg_with_arg(mp_msg_t msg, uint8_t arg);
//initialize and test connection; returns true if a connection is established
bool multiplayer_init_connection(uint32_t total_timeout_ms);

#endif