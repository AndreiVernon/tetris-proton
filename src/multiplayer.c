#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "multiplayer.h"

#define UART_INST uart1
#define UART_TX_PIN 40
#define UART_RX_PIN 41

#define HANDSHAKE_RETRY_MS 100
#define HANDSHAKE_TOTAL_TIMEOUT_MS 3000
#define DEFAULT_BAUD 115200

//init uart hardware
//note:change UART1_IRQ if you use uart0
#define UART_IRQ UART1_IRQ

//global simple state set by IRQ
volatile uint8_t garbage_add = 0;    //accumulate garbage lines from send_lines msg
volatile uint8_t last_msg_code = 0;  //lower nibble = msg code
volatile uint8_t last_arg = 0;       //upper nibble = arg
volatile bool msg_pending = false;   //set when IRQ writes last_msg_code/last_arg

//encode enum to 4-bit code
static inline uint8_t mp_encode(mp_msg_t m) {
	switch(m) {
		case mp_msg_ping:      return 0x01;
		case mp_msg_pong:      return 0x02;
		case mp_msg_send_lines:return 0x03;
		case mp_msg_game_over: return 0x04;
		case mp_msg_ack:       return 0x05;
		case mp_msg_sync:      return 0x06;
		default:               return 0x00;
	}
}

//decode 4-bit code to enum (unknown -> mp_msg_none)
static inline mp_msg_t mp_decode(uint8_t code) {
	switch(code & 0x0F) {
		case 0x01: return mp_msg_ping;
		case 0x02: return mp_msg_pong;
		case 0x03: return mp_msg_send_lines;
		case 0x04: return mp_msg_game_over;
		case 0x05: return mp_msg_ack;
		case 0x06: return mp_msg_sync;
		default:   return mp_msg_none;
	}
}

//send a raw byte (blocking)
bool multiplayer_send_byte(uint8_t b) {
	uart_putc_raw(UART_INST, b);
	return true;
}

//send single-message (lower nibble)
bool multiplayer_send_msg(mp_msg_t msg) {
	uint8_t b = mp_encode(msg) & 0x0F;
	return multiplayer_send_byte(b);
}

//send packed: upper4=arg lower4=msg
bool multiplayer_send_packed(uint8_t arg, mp_msg_t msg) {
	uint8_t code = mp_encode(msg) & 0x0F;
	uint8_t b = ((arg & 0x0F) << 4) | code;
	return multiplayer_send_byte(b);
}

//receive one raw byte with timeout in ms; kept for legacy/utility
bool multiplayer_receive_byte_timed(uint8_t *out, uint32_t timeout_ms) {
	if(!out) return false;

	absolute_time_t deadline = make_timeout_time_ms(timeout_ms);

	while (!time_reached(deadline)) {
		if (uart_is_readable(UART_INST)) {
			int c = uart_getc(UART_INST);
			if (c >= 0) {
				*out = (uint8_t)c;
				return true;
			}
		}
		sleep_ms(1);
	}

	return false;
}

//blocking drain of UART RX (useful during startup to clear noise)
void multiplayer_drain_rx(void) {
	while (uart_is_readable(UART_INST)) {
		(void)uart_getc(UART_INST);
	}
}

//get and clear accumulated garbage added by IRQ handler
uint8_t multiplayer_get_and_clear_garbage(void) {
	uint8_t v = garbage_add;
	garbage_add = 0;
	return v;
}

//consume last message (atomically get and clear pending flag)
//returns true if there was a message; out_code is lower nibble, out_arg is upper nibble
bool multiplayer_consume_last_message(uint8_t *out_code, uint8_t *out_arg) {
	if (!msg_pending) return false;
	//simple atomic-ish capture:disable IRQ briefly
	bool irq_was = irq_is_enabled(UART_IRQ);
	irq_set_enabled(UART_IRQ, false);

	if (out_code) *out_code = last_msg_code;
	if (out_arg)  *out_arg  = last_arg;
	msg_pending = false;

	irq_set_enabled(UART_IRQ, irq_was);
	return true;
}

//internal IRQ handler:reads all bytes available and updates globals
static void multiplayer_on_uart_irq(void) {
	//read all pending bytes
	while (uart_is_readable(UART_INST)) {
		int c = uart_getc(UART_INST);
		if (c < 0) continue;

		uint8_t b = (uint8_t)c;
		uint8_t arg = (b >> 4) & 0x0F; //upper nibble
		uint8_t code = b & 0x0F;       //lower nibble

		mp_msg_t m = mp_decode(code);

		//built-in tiny behaviours
		if (m == mp_msg_ping) {
			//reply pong immediately
			uart_putc_raw(UART_INST, mp_encode(mp_msg_pong) & 0x0F);
		} else if (m == mp_msg_send_lines) {
			//add to garbage counter
			garbage_add = (uint8_t)(garbage_add + arg);
		}

		//store last message for main to consume
		last_msg_code = code;
		last_arg = arg;
		msg_pending = true;
	}

	//note:reading RX FIFO typically clears IRQ source
}

//init uart hardware + enable RX interrupts and IRQ handler
void multiplayer_uart_init() {
	uart_init(UART_INST, DEFAULT_BAUD);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	//disable fifo thresholds/tweaks, use default settings
	uart_set_format(UART_INST, 8, 1, UART_PARITY_NONE);
	//no hw flow control
	uart_set_hw_flow(UART_INST, false, false);

	//install IRQ handler for RX-only
	irq_set_exclusive_handler(UART_IRQ, multiplayer_on_uart_irq);
	irq_set_enabled(UART_IRQ, true);
	uart_set_irq_enables(UART_INST, true, false);
}

//try handshake by sending ping and waiting for pong (one round)
static bool mp_try_handshake_round(uint32_t per_round_ms) {
	//send ping
	multiplayer_send_msg(mp_msg_ping);

	uint32_t wait_until_ms = to_ms_since_boot(get_absolute_time()) + per_round_ms;

	while ((int32_t)(wait_until_ms - to_ms_since_boot(get_absolute_time())) > 0) {
		if (uart_is_readable(UART_INST)) {
			int c = uart_getc(UART_INST);
			if (c < 0) continue;

			//decode lower nibble
			mp_msg_t r = mp_decode((uint8_t)c & 0x0F);

			//if we got a ping from other side, reply pong
			if (r == mp_msg_ping) {
				multiplayer_send_msg(mp_msg_pong);
			}

			//if we got a pong, handshake success
			if (r == mp_msg_pong) {
				//send ack so other side may also confirm
				multiplayer_send_msg(mp_msg_ack);
				return true;
			}
		}
		sleep_ms(1);
	}

	return false;
}

//init connection using repeated rounds until total timeout
bool multiplayer_init_connection(uint32_t total_timeout_ms) {
	absolute_time_t deadline = make_timeout_time_ms(total_timeout_ms);

	while (!time_reached(deadline)) {
		if (mp_try_handshake_round(HANDSHAKE_RETRY_MS)) return true;
	}

	return false;
}
