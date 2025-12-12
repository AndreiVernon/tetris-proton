#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "multiplayer.h"
#include "tetris.h"
#include "main.h"

#define UART_INST uart1
#define UART_TX_PIN 40
#define UART_RX_PIN 41

#define HANDSHAKE_RETRY_MS 100
#define HANDSHAKE_TOTAL_TIMEOUT_MS 3000
#define DEFAULT_BAUD 115200

//init uart hardware
//note:change UART1_IRQ if you use uart0
#define UART_IRQ UART1_IRQ

static volatile bool awaiting_ping_pong = false;
static volatile bool received_pong = false;

volatile bool grav_opt_received = false;
volatile uint8_t grav_opt_temp = 0;

volatile bool received_ping = false;
volatile bool mp_sync_ready = false;
volatile bool mp_sync_awaiting = false;
volatile int mp_pause_received = 0; //1 = pause, -1 = unpause
volatile bool mp_game_conn_received = false;

//send a raw byte (blocking)
static inline bool mp_send_byte(uint8_t b) {
	uart_putc_raw(UART_INST, b);
	return true;
}

//send single-message (lower nibble)
bool mp_send_msg(mp_msg_t msg) {
	uint8_t b = msg & 0x0F;
	return mp_send_byte(b);
}

//send packed: upper4=arg lower4=msg
bool mp_send_msg_packed(mp_msg_t msg, uint8_t arg) {
	uint8_t code = msg & 0x0F;
	uint8_t b = ((arg & 0x0F) << 4) | code;
	return mp_send_byte(b);
}

//blocking drain of UART RX (useful during startup to clear noise)
void mp_drain_rx() {
	while (uart_is_readable(UART_INST)) {
		(void)uart_getc(UART_INST);
	}
}

//internal IRQ handler
//reads all bytes available and updates globals
static void mp_on_uart_irq() {
	//read all pending bytes
	while (uart_is_readable(UART_INST)) {
		uint8_t msg_byte = uart_getc(UART_INST);
		uint8_t arg = (msg_byte >> 4) & 0x0F; //upper nibble
		uint8_t msg = msg_byte & 0x0F;       //lower nibble

        switch(msg) {
            case mp_msg_ping:
				//generic handshake
				if (arg == 0) {
					mp_send_msg(mp_msg_pong);
					if (awaiting_ping_pong) received_ping = true;
				}

				//sync
				if (arg == 2 && mp_sync_awaiting) {
					mp_send_msg_packed(mp_msg_pong, 2);
					mp_sync_ready = true;
				}

				if (arg == 3) {
					if (in_game) mp_game_conn_received = true;

					//if you quit after disconnection, quit other player too
					if (!in_game) mp_send_msg_packed(mp_msg_game_over, 2);
				}
                break;

            case mp_msg_pong:
				if (arg == 0 && awaiting_ping_pong) received_pong = true;
				if (arg == 2) mp_sync_ready = true;
                break;

            case mp_msg_send_lines:
				garbage_queue -= arg;
                break;

            case mp_msg_game_over:
				if (arg == 0) game_over = -1;
				if (arg == 2 && multiplayer) {
					game_over = 2;
				}
                break;

			case mp_msg_pause:
				if (multiplayer && in_game) {
					if (arg == 0) mp_pause_received = 1;
					if (arg == 1) mp_pause_received = -1;
				}
				break;

			case mp_msg_level_opt:
				if (multiplayer) {
					if (arg < start_level) start_level_effective = arg;
				}
				break;
			
			case mp_msg_grav_opt:
			case mp_msg_grav_opt_hi:
				if (!multiplayer) break;
				if (msg == mp_msg_grav_opt) grav_opt_temp |= arg;
				if (msg == mp_msg_grav_opt_hi) grav_opt_temp |= arg << 4;

				if (grav_opt_received) {
					if (arg > mp_level_timer) mp_level_timer_effective = arg;
				} else {
					grav_opt_received = true;
				}
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
	bool success = false;

	received_pong = false;
	received_ping = false;
	awaiting_ping_pong = true;
	mp_send_msg(mp_msg_ping);

	uint64_t wait_until_us = to_us_since_boot(get_absolute_time()) + timeout_us;

	while (to_us_since_boot(get_absolute_time()) < wait_until_us) {
		if (received_pong || received_ping) {
			success = true;
			break;
		}
	}

	received_pong = false;
	received_ping = false;
	awaiting_ping_pong = false;

	return success;
}
