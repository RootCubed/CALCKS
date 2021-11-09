#define F_CPU 8000000L
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

#include "../common/buttons.h"
#include "../common/gui.h"

const char BUTTON_MAP[35] = {
    off,        -1,      zero,         point,          enter,
	variable,   one,     two,          three,          plus,
	-1,         four,    five,         six,            minus,
	-1,         seven,   eight,        nine,           multiply,
	exponent,   -1,      bracket_open, bracket_close,  divide,
    btn_apps,   btn_sin, btn_cos,      btn_tan,        del,
    f1,         f2,      f3,           f4,
	back
};

void buttons_initialize() {
	DDRD = BUTTONS_SR_DATA; // SR pins as output
	DDRB |= BUTTONS_SR_CLEAR | BUTTONS_SR_CLK; // SR pins as output
	DDRD &= ~(BUTTONS_COL_0 | BUTTONS_COL_1 | BUTTONS_COL_2 | BUTTONS_COL_3 | BUTTONS_COL_4); // cols as input
	PORTD &= ~(BUTTONS_COL_0 | BUTTONS_COL_1 | BUTTONS_COL_2 | BUTTONS_COL_3 | BUTTONS_COL_4); // no pull-up

	DDRC &= ~(BUTTON_ON_OFF | BUTTON_UP | BUTTON_LEFT | BUTTON_RIGHT | BUTTON_DOWN);
	PORTC = (BUTTON_ON_OFF | BUTTON_UP | BUTTON_LEFT | BUTTON_RIGHT | BUTTON_DOWN);
	PORTC = 0xFF;

	PORTB &= ~BUTTONS_SR_CLEAR; // clear SR
	PORTB |= BUTTONS_SR_CLEAR;

	PORTB |= BUTTONS_SR_CLK; // clock high by default

	DDRB &= ~(_BV(PORTB0));
	PORTB |= _BV(PORTB0);
}

char buttons_get_special() {
	char res = PINC;
	return res;
}

int buttons_getPressed() {
	PORTB &= ~BUTTONS_SR_CLEAR; // clear SR
	PORTB |= BUTTONS_SR_CLEAR;
	
	// set first bit
	PORTD |= BUTTONS_SR_DATA;
	BUTTONS_SR_PULSE();
	// and remove it
	PORTD &= ~BUTTONS_SR_DATA;

	int res = -1;
	for (int i = 0; i < BUTTONS_NUM_ROWS; i++) {
		char pressed = PIND & (BUTTONS_COL_0 | BUTTONS_COL_1 | BUTTONS_COL_2 | BUTTONS_COL_3 | BUTTONS_COL_4);
		pressed >>= 2; // the column pins start at D2, not D0.
		if (PIND & (pressed << 2)) {
			int col = 0;
			while (((pressed >> col) & 1) == 0 && col < 5) col++;
			res = (BUTTONS_NUM_ROWS - i - 2) * 5 + col;
			break;
		}
		BUTTONS_SR_PULSE();
	}
	return res;
}