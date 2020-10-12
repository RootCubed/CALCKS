#define F_CPU 8000000L

#include "common/display.h"
#include "common/buttons.h"
#include "common/eep.h"
#include "common/gui.h"
#include "common/term.h"
#include "common/mandel.h"
#include "common/graph.h"

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
	#include <avr/sleep.h>
	#include <avr/interrupt.h>
#endif

enum modes {
	m_calc = 0,
	m_mandelbrot = 1,
	m_graph = 2
};

int cursorX = 0, cursorY = 0;
char currLine[0x20] = "";
char resBuf[0x20];
int lastButton = -1;
u8 currTerm[0x20];
opNode* termTree;

int currMode = m_calc;
int needsUpdating = 1;

void buttonPressed(int);

int main(void) {
	#ifdef console
		CreateThread(NULL, 0, ConsoleListener, NULL, 0, NULL);
	#endif
	buttons_initialize();
	disp_initialize();
	eep_initialize();
	
	gui_draw_image();

	#ifndef console
	char strBuf[16];
	sprintf(strBuf, "%02x", MCUSR);
	gui_draw_string(strBuf, 128 - 3 * 6, 64 - 6, FNT_SM, 0);
	MCUSR = 0;
	#endif
	
	_delay_ms(2000);
	
	disp_clear();
	buttons_initialize();

	#ifndef console
	// init ADC
	ADMUX = (0x01 << REFS0) /* AVCC with external capacitor at AREF pin */
			| (0 << ADLAR) /* Left Adjust Result: disabled */
			| (0b11110 << MUX0) /* 1.1V (Vbg) */;
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= (0b100 << ADPS0); // prescaler 16

	#endif
	
	int prevAdc = 0;
	float chargingAnim = 0.1;
	int chargingAnimDelay = 0;

	while (1) {
		//disp_clear();
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
				needsUpdating = 1;
			}
		} else {
			lastButton = -1;
		}

		long adc = 0;
		char strBuf[16];

		#ifndef console
		// calculate VCC
		ADCSRA |= _BV(ADSC); // start ADC conversion
		while (ADCSRA & _BV(ADIF)); // wait for conversion to complete
		adc = (ADCL | (ADCH << 8));
		double vcc = 1024 / (0.917 * adc) + 0.301 / 0.917; // more or less accurate
		
		if (adc != prevAdc || (PINB & 1) == 0) {
			if ((PINB & 1) == 0) {// charge controller PROG
				vcc = chargingAnim + 3.2; // make charging animation
				chargingAnim += 0.1;
				if (chargingAnim > 1) chargingAnim = 0.1;
				if (chargingAnimDelay < 500) {
					chargingAnimDelay++;
					goto ifLoopBreak;
				}
			} else {
				chargingAnim = 0.1;
			}
			chargingAnimDelay = 0;
			gui_draw_string(strBuf, 128 - 13 * 6, 49, 0, 0);
			double percentage = (vcc - 3.2) / (4.2 - 3.2); // interpolate linearly between 3.2 and 4.2
			
			gui_draw_rect(104, 0, 22, 6, 0);
			gui_draw_line(127, 2, 127, 4);
			for (int i = 0; i < (int) (percentage * 22); i++) {
				gui_draw_line(104 + i, 0, 104 + i, 6);
			}
			gui_clear_rect(104 + (int) (percentage * 22), 1, 22 - ((int) (percentage * 22)), 4);
		}
		ifLoopBreak:
		prevAdc = adc;
		#endif

		if (needsUpdating) {
			switch (currMode) {
				case m_calc:
					needsUpdating = 0;
					gui_tab_button("Menu", 0);
					gui_tab_button("Graph", 28);
					gui_tab_button("Mandel", 62);
					break;
				case m_mandelbrot:
					mandel_draw();
					needsUpdating = 0;
					break;
				case m_graph:
					termTree = parse_term(currTerm);
					graph_draw(termTree);
					term_free(termTree);
					needsUpdating = 0;
					break;
			}
		}

		_delay_ms(1);
	}
}

void buttonPressed(int buttonID) {
	if (buttonID != 15 && currTerm[cursorX] == CHAR_END) {
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
		termTree = parse_term(currTerm);
		double res = evaluate_term(termTree, 0);
		term_free(termTree);
		char resBuf[16];
		sprintf(resBuf, "%.3f", res);
		gui_draw_string(resBuf, 0, 16, FNT_MD, 0);
	}
	if (buttonID == 15) {
		// put display into sleep mode (turn display off, turn all points on)

		#ifndef console
		disp_command(DISP_CMD_ONOFF     | 0);
		disp_command(DISP_CMD_ALL_ONOFF | 1);

		// enable pin change interrupt for PC2 / PCINT18
		PCICR = 0b0100;
		PCMSK2 = 0b00000100; // PCINT8
		
		sei();
		set_sleep_mode(0b101);
		sleep_enable();
		sleep_cpu();

		//after wake up, disable SE and disable interrupts
		sleep_disable();
		cli();

		// turn display back on
		disp_command(DISP_CMD_ONOFF     | 1);
		disp_command(DISP_CMD_ALL_ONOFF | 0);
		#endif
	}
	if (buttonID == 16) {
		currMode = m_mandelbrot;
	}
	if (buttonID == 17) {
		mandel_reset_state();
		graph_reset_state();
		currMode = m_calc;
		disp_clear();
		needsUpdating = 1;
	}
	if (buttonID == 18) {
		currMode = m_graph;
		currTerm[cursorX] = CHAR_END;
	}
	if (buttonID == 19) {
		currTerm[cursorX] = VAR_X;
		gui_draw_char(cursorX * 8, 0, 59, FNT_MD, 1);
		cursorX++;
	}
	if (buttonID == 20) {
		currTerm[cursorX] = OP_BRACK_OPEN;
		gui_draw_char(cursorX * 8, 0, 68, FNT_MD, 1);
		cursorX++;
	}
	if (buttonID == 21) {
		currTerm[cursorX] = OP_BRACK_CLOSE;
		gui_draw_char(cursorX * 8, 0, 69, FNT_MD, 1);
		cursorX++;
	}
}

#ifndef console
ISR(PCINT2_vect) {}
#endif