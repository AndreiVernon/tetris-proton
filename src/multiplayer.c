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
void multiplayer_uart_init() {
	uart_init(UART_INST, DEFAULT_BAUD);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	//disable fifo thresholds/tweaks, use default settings
	uart_set_format(UART_INST, 8, 1, UART_PARITY_NONE);
	//no hw flow control
	uart_set_hw_flow(UART_INST, false, false);
	//set read/write blocking timeouts not required (we poll)
}

//encode enum to byte
static inline uint8_t mp_encode(mp_msg_t m) {
	return (uint8_t)m;
}

//decode byte to enum (unknown maps to mp_msg_none)
static inline mp_msg_t mp_decode(uint8_t b) {
	switch(b) {
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

//send a single-message code
bool multiplayer_send_msg(mp_msg_t msg) {
	uint8_t b = mp_encode(msg);
	return multiplayer_send_byte(b);
}

//send a message with an extra 1-byte argument
bool multiplayer_send_msg_with_arg(mp_msg_t msg, uint8_t arg) {
	uart_putc_raw(UART_INST, mp_encode(msg));
	uart_putc_raw(UART_INST, arg);
	return true;
}

//receive one raw byte with timeout in ms; returns true on success and sets *out
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
		//small sleep to avoid busy spin
		sleep_ms(1);
	}
	return false;
}

//receive one message (single-byte code) with timeout_ms; returns true on success
bool multiplayer_receive_msg_timed(mp_msg_t *out_msg, uint32_t timeout_ms) {
	if(!out_msg) return false;
	uint8_t b;
	bool ok = multiplayer_receive_byte_timed(&b, timeout_ms);
	if(!ok) return false;
	*out_msg = mp_decode(b);
	return (*out_msg != mp_msg_none);
}

//try handshake by sending ping and waiting for pong
static bool mp_try_handshake_round(uint32_t per_round_ms) {
	//send ping
	multiplayer_send_msg(mp_msg_ping);
	//wait briefly for anything
	uint32_t wait_until_ms = to_ms_since_boot(get_absolute_time()) + per_round_ms;
	while ((int32_t)(wait_until_ms - to_ms_since_boot(get_absolute_time())) > 0) {
		if (uart_is_readable(UART_INST)) {
			int c = uart_getc(UART_INST);
			if (c < 0) continue;
			mp_msg_t r = mp_decode((uint8_t)c);
			//if we got a ping from other side, reply pong
			if (r == mp_msg_ping) {
				multiplayer_send_msg(mp_msg_pong);
			}
			//if we got a pong, handshake success
			if (r == mp_msg_pong) {
				//send ack so other side may also confirm (optional)
				multiplayer_send_msg(mp_msg_ack);
				return true;
			}
		}
		sleep_ms(1);
	}
	return false;
}

//initialize and test connection; returns true if a connection is established
//both sides run this and should succeed
bool multiplayer_init_connection(uint32_t total_timeout_ms) {
	//attempt repeated ping->wait-for-pong, while also replying pong if peer pings
	absolute_time_t deadline = make_timeout_time_ms(total_timeout_ms);
	while (!time_reached(deadline)) {
		if (mp_try_handshake_round(HANDSHAKE_RETRY_MS)) return true;
		//keep trying until timeout
	}
	return false;
}

//automatic connection self-test; returns true if loopback style test passes
//it will send a ping and expect a pong within timeout_ms
bool multiplayer_connection_test(uint32_t timeout_ms) {
	//send ping and wait for pong
	multiplayer_send_msg(mp_msg_ping);
	mp_msg_t r;
	uint32_t start_ms = to_ms_since_boot(get_absolute_time());
	while ((to_ms_since_boot(get_absolute_time()) - start_ms) < timeout_ms) {
		if (uart_is_readable(UART_INST)) {
			int c = uart_getc(UART_INST);
			if (c < 0) continue;
			r = mp_decode((uint8_t)c);
			if (r == mp_msg_ping) {
				//peer pinged us, respond with pong
				multiplayer_send_msg(mp_msg_pong);
			}
			if (r == mp_msg_pong) {
				//got pong, test ok
				return true;
			}
		}
		sleep_ms(1);
	}
	return false;
}

//decode-and-handle routine
//returns true if message was recognized and handled
bool multiplayer_handle_incoming_once(uint32_t timeout_ms) {
	mp_msg_t msg;
	if (!multiplayer_receive_msg_timed(&msg, timeout_ms)) return false;
	switch(msg) {
		case mp_msg_ping:
			//reply pong
			multiplayer_send_msg(mp_msg_pong);
			return true;
		case mp_msg_pong:
			return true;
		case mp_msg_send_lines:
			return true;
		case mp_msg_game_over:
			return true;
		default:
			return false;
	}
}

//blocking drain of UART RX (useful during startup to clear noise)
void multiplayer_drain_rx(void) {
	while (uart_is_readable(UART_INST)) {
		(void)uart_getc(UART_INST);
	}
}

//read one extra arg byte (if protocol uses it)
bool multiplayer_receive_arg_byte(uint8_t *arg, uint32_t timeout_ms) {
	return multiplayer_receive_byte_timed(arg, timeout_ms);
}
