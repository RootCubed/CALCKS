#define F_CPU 8000000L

#include "common/display.h"
#include "common/buttons.h"
#include "common/eep.h"
#include "common/gui.h"
#include "common/term.h"

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
char currLine[0x20] = "";
char resBuf[0x20];
int lastButton = -1;
u8 currTerm[0x20];

void buttonPressed(int);

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

	#ifndef console
	// init ADC
	ADMUX = (0x01 << REFS0) /* AVCC with external capacitor at AREF pin */
			| (0 << ADLAR) /* Left Adjust Result: disabled */
			| (0b11110 << MUX0) /* 1.1V (Vbg) */;
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= (0b100 << ADPS0); // prescaler 16
	#endif
	
	while (1) {
		buttons_get_special();
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
		#ifndef console
		// calculate VCC
		long adc = 0;
		ADCSRA |= _BV(ADSC); // start ADC conversion
		while (ADCSRA & _BV(ADIF)); // wait for conversion to complete
		adc = (ADCL | (ADCH << 8));
		double vcc = 1024 / (0.917 * adc) + 0.301 / 0.917;
		char vccBuf[16];
		sprintf(vccBuf, "Vcc %ld    %.3fV", adc, vcc);
		gui_draw_string(vccBuf, 0, 32, FNT_SM, 0);
		#endif
		_delay_ms(20);
	}
}

void buttonPressed(int buttonID) {
	if (currTerm[cursorX] == CHAR_END) {
		for (int i = 0; i < 0x80; i++) currTerm[i] = 0;
		cursorX = 0;
		disp_clear();
	}
	if (buttonID < 10) {
		currTerm[cursorX] = buttonID;
		gui_draw_char(cursorX * 8, 0, buttonID, FNT_MD, 1);
		cursorX++;
	} else {
		if (buttonID < 14) {
			currTerm[cursorX] = (buttonID - 10) | (OPTYPE_SIMPLE << 5);
			gui_draw_char(cursorX * 8, 0, 62 - 10 + buttonID, FNT_MD, 1);
			cursorX++;
		}
	}
	if (buttonID == 14) {
		currTerm[cursorX] = CHAR_END;
		opNode* termTree = parse_term(currTerm);
		double res = evaluate_term(termTree);
		term_free(termTree);
		char resBuf[16];
		sprintf(resBuf, "%.3f", res);
		gui_draw_string(resBuf, 0, 16, FNT_MD, 0);
	}
}