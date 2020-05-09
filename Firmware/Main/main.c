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
int resBuf[0x80];
int lastButton = -1;

void buttonPressed(int);
void doCalculation(char*, int*);

int main(void) {
	disp_initialize();
	buttons_initialize();
	eep_initialize();
	
	disp_clear();
	
	disp_drawImage();
	
	_delay_ms(2000);
	
	disp_clear();
	
	for (int i = 0; i < 70; i++) {
		disp_drawChar((i * 8) % 128, i / 16 * 16, i, FNT_SM);
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
	if (currLine[cursorX] == 255) {
		for (int i = 0; i < 0x80; i++) currLine[i] = 0;
		cursorX = 0;
		disp_clear();
	}
	if (buttonID < 10) {
		currLine[cursorX] = buttonID;
		disp_drawChar(cursorX * 8, 0, currLine[cursorX], FNT_MD);
		cursorX++;
	} else {
		if (buttonID < 14) {
			currLine[cursorX] = 62 - 10 + buttonID;
			disp_drawChar(cursorX * 8, 0, currLine[cursorX], FNT_MD);
			cursorX++;
		}
		if (buttonID == 14) {
			currLine[cursorX] = 255;
			doCalculation(currLine, resBuf);
			int i = 0;
			while (resBuf[i] != 0) {
				disp_drawChar(i * 8, 16, resBuf[i], FNT_MD);
				i++;
			}
		}
	}
}

void doCalculation(char* input, int* output) {
	int i = 0;
	long result = 0;
	int isFirstNum = 1;
	while (input[i] != 255) {
		long bufNum = 0;
		int numLength = 0;
		while (input[i] < 10) {
			numLength++;
			i++;
		}
		long power = pow(10, numLength);
		for (int j = i - numLength; j < i; j++) {
			bufNum += (int) input[j] * power;
			power /= 10;
		}
		if (isFirstNum) {
			result = bufNum;
			isFirstNum = 0;
		} else {
			switch (input[i]) {
				case 62:
				result += bufNum;
				break;
				case 63:
				result -= bufNum;
				break;
				case 64:
				result /= bufNum;
				break;
				case 65:
				result *= bufNum;
				break;
			}
		}
		i++;
	}
	int j = 0;
	output[j] = 9;
	while (result >= 0) {
		output[j] = result % 10;
		result /= 10;
		j++;
		if (result == 0) break;
	}
	output[j] = 0;
}

