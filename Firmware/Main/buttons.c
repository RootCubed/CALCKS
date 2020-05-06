/*
 * buttons.c
 *
 * Created: 20.03.2020 17:30:52
 *  Author: liamb
 */ 

#define F_CPU 16000000L
#include <util/delay.h>

#include <avr/io.h>
#include "buttons.h"

void buttons_initialize() {
	DDRD = BUTTONS_SR_DATA | BUTTONS_SR_CLEAR | BUTTONS_SR_CLK; // SR pins as output
	DDRD &= ~BUTTONS_ROW_0 & ~BUTTONS_ROW_1 & ~BUTTONS_FUNC_SWITCH; // rows as input

	PORTD &= ~BUTTONS_SR_CLEAR; // clear SR
	PORTD |= BUTTONS_SR_CLEAR;

	PORTD |= BUTTONS_SR_CLK; // clock high by default
}

int buttons_getPressed() {
	PORTD &= ~BUTTONS_SR_CLEAR; // clear SR
	PORTD |= BUTTONS_SR_CLEAR;
	
	// set first bit
	PORTD |= BUTTONS_SR_DATA;
	BUTTONS_SR_PULSE();
	// and remove it
	PORTD &= ~BUTTONS_SR_DATA;

	int i;
	int res = -1;
	for (i = BUTTONS_NUM_COLUMNS; i >= 0; i--) {
		if (PIND & (BUTTONS_ROW_0 | BUTTONS_ROW_1)) {
			res = (BUTTONS_NUM_COLUMNS - i) + (((PIND & BUTTONS_ROW_0) != 0) ? 0 : BUTTONS_NUM_COLUMNS);
			break;
		}
		BUTTONS_SR_PULSE();
	}
	if (res > -1) {
		return res + (PIND & BUTTONS_FUNC_SWITCH) * 10;
	}
	return -1;
}
