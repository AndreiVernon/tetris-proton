#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "multiplayer.h"
#include "tetris.h"

#define UART_INST uart1
#define UART_TX_PIN 40
#define UART_RX_PIN 41

#define HANDSHAKE_RETRY_MS 100
#define HANDSHAKE_TOTAL_TIMEOUT_MS 3000
#define DEFAULT_BAUD 115200

//init uart hardware
//note:change UART1_IRQ if you use uart0
#define UART_IRQ UART1_IRQ

static bool awaiting_pong = false;
static bool received_pong = false;

volatile bool received_ping = false;
volatile bool mp_sync_ready = false;
volatile int mp_pause_received = 0; //1 = pause, -1 = unpause

//encode enum to 4-bit code
static inline uint8_t mp_encode(mp_msg_t m) {
	switch(m) {
		case mp_msg_ping:      return 0x01;
		case mp_msg_pong:      return 0x02;
		case mp_msg_send_lines:return 0x03;
		case mp_msg_game_over: return 0x04;
		case mp_msg_pause:	   return 0x05;
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
		case 0x05: return mp_msg_pause;
		default:   return mp_msg_none;
	}
}

//send a raw byte (blocking)
static inline bool mp_send_byte(uint8_t b) {
	uart_putc_raw(UART_INST, b);
	return true;
}

//send single-message (lower nibble)
bool mp_send_msg(mp_msg_t msg) {
	uint8_t b = mp_encode(msg) & 0x0F;
	return mp_send_byte(b);
}

//send packed: upper4=arg lower4=msg
bool mp_send_msg_packed(mp_msg_t msg, uint8_t arg) {
	uint8_t code = mp_encode(msg) & 0x0F;
	uint8_t b = ((arg & 0x0F) << 4) | code;
	return mp_send_byte(b);
}

//blocking drain of UART RX (useful during startup to clear noise)
static inline void mp_drain_rx(void) {
	while (uart_is_readable(UART_INST)) {
		(void)uart_getc(UART_INST);
	}
}

//internal IRQ handler
//reads all bytes available and updates globals
static void mp_on_uart_irq(void) {
	if (!multiplayer) return;

	//read all pending bytes
	while (uart_is_readable(UART_INST)) {
		uint8_t msg_byte = uart_getc(UART_INST);
		uint8_t arg = (msg_byte >> 4) & 0x0F; //upper nibble
		uint8_t code = msg_byte & 0x0F;       //lower nibble

		mp_msg_t msg = mp_decode(code);

        switch(msg) {
            case mp_msg_ping:
				if (arg == 2) {
					mp_send_msg_packed(mp_msg_pong, 2);
					mp_sync_ready = true;
				}
				else mp_send_msg(mp_msg_pong);

				received_ping = true;
                break;

            case mp_msg_pong:
				if (awaiting_pong) received_pong = true;
				if (arg == 2) mp_sync_ready = true;
                break;

            case mp_msg_send_lines:
				garbage_queue += arg;
                break;

            case mp_msg_game_over:
				game_over = -1;
                break;

			case mp_msg_pause:
				if (arg == 0) mp_pause_received = 1;
				if (arg == 1) mp_pause_received = -1;
				break;

            default:               
                break;
        }
	}
}

void mp_uart_init() {
	uart_init(UART_INST, DEFAULT_BAUD);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	//disable fifo thresholds/tweaks, use default settings
	uart_set_format(UART_INST, 8, 1, UART_PARITY_NONE);
	//no hw flow control
	uart_set_hw_flow(UART_INST, false, false);

    mp_drain_rx();

	//install IRQ handler for RX-only
	irq_set_exclusive_handler(UART_IRQ, mp_on_uart_irq);
	irq_set_enabled(UART_IRQ, true);
	uart_set_irq_enables(UART_INST, true, false);
}

//try handshake by sending ping and waiting for pong
bool mp_handshake_blocking(uint64_t timeout_us) {
	mp_send_msg(mp_msg_ping);

	awaiting_pong = true;
	received_pong = false;

	uint64_t wait_until_us = to_us_since_boot(get_absolute_time()) + timeout_us;

	while (to_us_since_boot(get_absolute_time()) < wait_until_us && !received_pong) {
		tight_loop_contents();
	}

	awaiting_pong = false;
	return received_pong;
}
