/*
 * Calculator.c
 *
 * Created: 19.03.2020 14:47:42
 * Author : liamb
 */

#define F_CPU 16000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "display.h"
#include "buttons.h"
#include "../EEPROM/eep.h"

int cursorX = 0, cursorY = 0;
char currLine[0x80] = "";
int lastButton = -1;

void buttonPressed(int);

int main(void) {
	disp_initialize();
	buttons_initialize();
	eep_initialize();
	
	disp_clear();
	
	disp_drawImage();
	
	_delay_ms(2000);
	
	disp_clear();
	
	for (int i = 0; i < 62; i++) {
		disp_drawChar((i * 8) % 128, i / 16 * 16, i, FNT_MD);
	}
	
	_delay_ms(2000);
	
	disp_clear();
	
	while (1) {
		int currButton = buttons_getPressed();
		if (currButton != -1) {
			if (currButton != lastButton) {
				lastButton = currButton;
				buttonPressed(currButton);
			}
		} else {
			lastButton = -1;
		}
		_delay_ms(20);
	}
}

void buttonPressed(int buttonID) {
	currLine[cursorX] = '0' + buttonID;
	disp_drawChar(cursorX * 8, 0, currLine[cursorX] - '0', FNT_MD);
	cursorX++;
}

