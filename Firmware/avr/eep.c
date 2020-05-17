/*
 * eep.c
 *
 * Created: 20.03.2020 22:23:23
 *  Author: liamb
 */

#include <avr/io.h>
#include <util/twi.h>
#include <stdlib.h>

#include <stdio.h>

#include "../common/eep.h"

#define F_CPU 16000000UL
#define BAUD 9600

#include <util/delay.h>
#include <util/setbaud.h>

char lolBuf[256];
char flBuf[5];

int counter = 0;

void uart_init(void) {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

char uart_getbyte() {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

void uart_putbyte(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

void uart_print(const char *str) {
	int ptr = 0;
	while (str[ptr] != '\0') {
		uart_putbyte(str[ptr]);
		ptr++;
	}
}

void eep_initialize() {
	uart_init();
	//set SCL to 400kHz
	TWSR = 0x00;
	TWBR = 0x0C;
	TWCR = _BV(TWEN);
}

void i2c_action(unsigned char control) {
	TWCR = _BV(TWINT) | control;
	while((TWCR & _BV(TWINT)) == 0);
	/*_delay_ms(10);
	snprintf(lolBuf, 256, "0x%02x\r\n", TWCR & 0xF8);
	uart_print(lolBuf);*/
}

void i2c_begin() {
	//start
	i2c_action(_BV(TWSTA) | _BV(TWEN));
}

int i2c_write(unsigned char data) {
	// send the data
	TWDR = data;
	i2c_action(_BV(TWEN));
	return 0;
}

char i2c_read(int sendACK) {
	// read the data
	i2c_action(_BV(TWEN) | (sendACK ? 0 : _BV(TWEA)));
	return TWDR;
}

void i2c_end() {
	// set the stop condition
	TWCR = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTO));
}

void eep_write_block(const char *src, int position, int length) {
	int lengthOld = length;
	int arrPos = 0;
	while (length > 0) {
		i2c_begin();
		i2c_write(0b10100000 | TW_WRITE);
		i2c_write(position >> 8); // MSB
		i2c_write(position & 0xFF); // LSB
		
		int numBytes;
		if (length > 8) numBytes = 8;
		else numBytes = length;
		
		for (int i = 0; i < numBytes; i++) {
			i2c_write(src[arrPos + i]);
			length--;
		}
		i2c_end();
		position += 8;
		arrPos += numBytes;
		_delay_ms(10);
	}
}

void eep_read_block(void *dst, int position, int length) {
	i2c_begin();
	i2c_write(0b10100000 | TW_WRITE);
	i2c_write(position >> 8); // MSB
	i2c_write(position & 0xFF); // LSB
	i2c_begin();
	i2c_write(0b10100000 | TW_READ);
	int sendNAK = 0;
	for (int i = 0; i < length; i++) {
		if (i == length - 1) {
			sendNAK = 1;
		}
		((char *)dst)[i] = i2c_read(sendNAK);
	}
	i2c_end();
}