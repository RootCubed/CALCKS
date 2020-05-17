/*
 * EEPROMWrite.c
 *
 * Created: 19.03.2020 18:08:41
 * Author : liamb
 */ 

#define F_CPU 16000000L

#include <avr/io.h>
#include <util/delay.h>

#include "common/eep.h"

#define BUFSIZE 1024

int main(void)
{
	DDRC = 0xFF;
	PORTC &= ~_BV(PORTC0);
	PORTC &= ~_BV(PORTC1);
	eep_initialize();
	uart_print("s");
	// get length of data
	char hi = uart_getbyte();
	char lo = uart_getbyte();
	int length = (hi << 8) | lo;

	int position = 0;
	char buffer[BUFSIZE];
	while (position < length) {
		if ((position % BUFSIZE) == 0) {
			if (position > 0) {
				PORTC |= _BV(PORTC0);
				eep_write_block(buffer, position - BUFSIZE, 0x400);
				PORTC &= ~_BV(PORTC0);
			}
			uart_print("n");
		}
		buffer[position % BUFSIZE] = uart_getbyte();
		position++;
	}
	uart_print("d");
	eep_write_block(buffer, position - (position % BUFSIZE), position % BUFSIZE);
	PORTC |= _BV(PORTC0);
	while (1) {
		PORTC |= _BV(PORTC0);
		_delay_ms(50);
		PORTC &= ~_BV(PORTC0);
		_delay_ms(50);
	}
}