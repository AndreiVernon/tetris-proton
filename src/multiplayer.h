#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__

#include "pico/stdlib.h"

typedef enum {
    mp_msg_none        = 0x00,
	mp_msg_ping        = 0x01,
	mp_msg_pong        = 0x02,
	mp_msg_send_lines  = 0x03,
	mp_msg_game_over   = 0x04,
} mp_msg_t;

#endif