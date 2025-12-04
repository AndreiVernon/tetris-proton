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

static bool awaiting_pong;
static bool received_pong;

//encode enum to 4-bit code
static inline uint8_t mp_encode(mp_msg_t m) {
	switch(m) {
		case mp_msg_ping:      return 0x01;
		case mp_msg_pong:      return 0x02;
		case mp_msg_send_lines:return 0x03;
		case mp_msg_game_over: return 0x04;
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
bool mp_send_msg_packed(uint8_t arg, mp_msg_t msg) {
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
	//read all pending bytes
	while (uart_is_readable(UART_INST)) {
		uint8_t msg_byte = uart_getc(UART_INST);
		uint8_t arg = (msg_byte >> 4) & 0x0F; //upper nibble
		uint8_t code = msg_byte & 0x0F;       //lower nibble

		mp_msg_t msg = mp_decode(code);

        switch(msg) {
            case mp_msg_ping:
                mp_send_msg(mp_msg_pong);
                break;
            case mp_msg_pong:
				if (awaiting_pong) received_pong = true;
                break;
            case mp_msg_send_lines:
                break;
            case mp_msg_game_over:
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
bool mp_handshake_blocking(uint32_t timeout_ms) {
	mp_send_msg(mp_msg_ping);

	awaiting_pong = true;
	received_pong = false;

	uint32_t wait_until_ms = to_ms_since_boot(get_absolute_time()) + timeout_ms;

	while (to_ms_since_boot(get_absolute_time()) < wait_until_ms && !received_pong) {
		tight_loop_contents();
	}

	awaiting_pong = false;
	return received_pong;
}
