#define F_CPU 8000000L
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

#include "../common/buttons.h"
#include "../common/gui.h"

const char BUTTON_MAP[30] = {
    14, // enter
    -1, // button doesn't work
    -1, 0, -1,
    10, // plus
    -1, // button doesn't work
    1, 2, 3,
    11, // minus
    -1, // button doesn't work
    4, 5, 6,
    12, // div
    -1, // button doesn't work
    7, 8, 9,
    13, // mult
    -1, -1, -1, -1
};

void buttons_initialize() {
	DDRD = BUTTONS_SR_DATA; // SR pins as output
	DDRB |= BUTTONS_SR_CLEAR | BUTTONS_SR_CLK; // SR pins as output
	DDRD &= ~(BUTTONS_ROW_0 | BUTTONS_ROW_1 | BUTTONS_ROW_2 | BUTTONS_ROW_3 | BUTTONS_ROW_4); // rows as input
	PORTD &= ~(BUTTONS_ROW_0 | BUTTONS_ROW_1 | BUTTONS_ROW_2 | BUTTONS_ROW_3 | BUTTONS_ROW_4); // no pull-up

	PORTB &= ~BUTTONS_SR_CLEAR; // clear SR
	PORTB |= BUTTONS_SR_CLEAR;

	PORTB |= BUTTONS_SR_CLK; // clock high by default
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
	char buf[16];
	for (int i = 0; i < BUTTONS_NUM_COLUMNS; i++) {
		char pressed = PIND & (BUTTONS_ROW_0 | BUTTONS_ROW_1 | BUTTONS_ROW_2 | BUTTONS_ROW_3 | BUTTONS_ROW_4);
		sprintf(buf, "%02x", pressed);
		gui_drawString(buf, 50, 8 * i, 0, 0);
		if (PIND & pressed) {
			int row = 0;
			while (((pressed >> row) & 1) == 0 && row < 5) row++;
			res = (BUTTONS_NUM_COLUMNS - i - 1) * 5 + row;
			//break;
		}
		BUTTONS_SR_PULSE();
	}
	sprintf(buf, "%02d", res);
	gui_drawString(buf, 100, 60, 0, 0);
	return res;
}