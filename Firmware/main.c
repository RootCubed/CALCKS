#define F_CPU 8000000L

#include "common/display.h"
#include "common/buttons.h"
#include "common/eep.h"
#include "common/gui.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef console
	#include <windows.h>
	#define _delay_ms Sleep
	DWORD WINAPI ConsoleListener(void* data) {
		while (1) {
			char code = getchar();
			if (code == 'g') {
				disp_print();
			}
			if (code == 'b') {
				char button;
				scanf("%d", &button);
				set_button(button);
			}
		}
		return 0;
	}
#else
	#include <avr/io.h>
	#include <util/delay.h>
#endif

int cursorX = 0, cursorY = 0;
char currLine[0x80] = "";
char resBuf[0x80];
int lastButton = -1;

void buttonPressed(int);
void doCalculation(char*, char*);

int main(void) {
	#ifdef console
		CreateThread(NULL, 0, ConsoleListener, NULL, 0, NULL);
	#endif
	buttons_initialize();
	disp_initialize();
	eep_initialize();
	
	disp_clear();

	gui_draw_image();
	
	_delay_ms(2000);
	
	for (int i = 0; i < 70; i++) {
		gui_draw_char((i * 8) % 128, i / 16 * 16, i, FNT_SM, 0);
	}
	
	disp_clear();
	buttons_initialize();
	int xTabPos = 4;
	xTabPos += gui_tab_button("Menu", 4);
	xTabPos += gui_tab_button("Graph", xTabPos + 2);
	
	while (1) {
		int btnUnmapped = buttons_getPressed();
		int currButton = -1;
		if (btnUnmapped > -1) {
			currButton = BUTTON_MAP[btnUnmapped];
		}
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
	if (currLine[cursorX] == -1) {
		for (int i = 0; i < 0x80; i++) currLine[i] = 0;
		cursorX = 0;
		disp_clear();
	}
	if (buttonID < 10) {
		currLine[cursorX] = buttonID;
		gui_draw_char(cursorX * 8, 0, currLine[cursorX], FNT_MD, 1);
		cursorX++;
	} else {
		if (buttonID < 14) {
			currLine[cursorX] = 62 - 10 + buttonID;
			gui_draw_char(cursorX * 8, 0, currLine[cursorX], FNT_MD, 1);
			cursorX++;
		}
		if (buttonID == 14) {
			currLine[cursorX] = -1;
			doCalculation(currLine, resBuf);
			gui_draw_string(resBuf, 0, 16, FNT_MD, 0);
		}
	}
}

long extrNum(char *input, int* pos) {
	long bufNum = 0;
	while (input[*pos] > -1 && input[*pos] < 10) {
		bufNum = (bufNum * 10) + input[*pos];
		(*pos)++;
	}
	return bufNum;
}

void doCalculation(char* input, char* output) {
	int i = 0;
	double result = 0;
	int isFirstNum = 1;
	while (input[i] > -1) {
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
	sprintf(resBuf, "%f", result);
}

