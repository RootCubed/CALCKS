#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>

#define F_CPU 8000000UL
#define BAUD 9600

#include <util/delay.h>
#include <util/setbaud.h>

#include "../common/uart.h"

void uart_init(void) {
	DDRD &= ~_BV(PD0);

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	UCSR0A &= ~(_BV(U2X0));

	UCSR0B = _BV(RXEN0) | _BV(RXCIE0);
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);

	uart_wasRead = 1;

	sei();
}

int uartwritepos = 0;

ISR(USART0_RX_vect) {
	char c = UDR0;
	if (uart_wasRead == 1) {
		uart_buf[uartwritepos] = c;
		uartwritepos++;
		if (uartwritepos == UART_BUFSIZE || c == '\n') {
			uart_buf[uartwritepos - 1] = '\0';
			uartwritepos = 0;
			uart_wasRead = 0;
		}
	}
}