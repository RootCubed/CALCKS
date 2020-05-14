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
			while (resBuf[i] != 255) {
				disp_drawChar(i * 8, 16, resBuf[i], FNT_MD);
				i++;
			}
		}
	}
}

long extrNum(char *input, int* pos) {
	long bufNum = 0;
	while (input[*pos] < 10) {
		bufNum = (bufNum * 10) + input[*pos];
		(*pos)++;
	}
	return bufNum;
}

void doCalculation(char* input, int* output) {
	int i = 0;
	long result = 0;
	int isFirstNum = 1;
	while (input[i] != 255) {
		if (isFirstNum == 1) {
			result = extrNum(input, &i);
			isFirstNum = 0;
		} else {
			char operand = input[i];
			i++;
			long num2 = extrNum(input, &i);
			switch (operand) {
				case 62:
				result += num2;
				break;
				case 63:
				result -= num2;
				break;
				case 64:
				result /= num2;
				break;
				case 65:
				result *= num2;
				break;
			}
		}
	}
	int len = floor(log10(result));
	int p = 0;
	do {
		output[len - p] = result % 10;
		result /= 10;
		p++;
	} while (result > 0);
	output[p] = 255;
}

