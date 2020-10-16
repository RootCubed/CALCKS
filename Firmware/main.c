#define F_CPU 8000000L

#define VERSION_STRING "v0.9.0"

#include "common/display.h"
#include "common/buttons.h"
#include "common/eep.h"
#include "common/gui.h"
#include "common/term.h"
#include "common/mandel.h"
#include "common/graph.h"
#include "common/mathinput.h"
#include "common/solver.h"

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
	m_calc,
	m_mandelbrot,
	m_graph,
	m_solve_menu,
	m_info
};

char resBuf[0x20];
int lastButton = -1;
opNode *termTree;

char strBuf[16];

inputBox *mainScreenInput;
int needsClearRes = 0;

int currMode = m_calc;
int needsRedraw = 1;

int prevAdc = 0;
float chargingAnim = 0.2;
int chargingAnimDelay = 50;

void buttonPressed(int);
void infoScreen_battery();

int main(void) {
	#ifdef console
		CreateThread(NULL, 0, ConsoleListener, NULL, 0, NULL);
	#endif
	buttons_initialize();
	disp_initialize();
	eep_initialize();
	
	gui_draw_image();
	
	_delay_ms(2000);
	
	disp_clear();
	buttons_initialize();

	mainScreenInput = mathinput_initBox(FNT_MD, 100, 0, 0);
	solver_init();

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

		switch (currMode) {
			case m_calc:
				if (needsRedraw) {
					gui_tab_button("Info", 0);
					gui_tab_button("Graph", 28);
					gui_tab_button("Mandel", 61);
					gui_tab_button("Solv", 100);
				}
				needsRedraw = 0;
				break;
			case m_mandelbrot:
				mandel_draw();
				break;
			case m_graph:
				graph_draw(mainScreenInput->buffer);
				break;
			case m_solve_menu:
				solver_updateScreen();
				break;
			case m_info:
				if (needsRedraw) {
					disp_clear();
					prevAdc = 0;
					chargingAnimDelay = 50;
					gui_draw_string("CALCKS", 5, 64 - 32, FNT_MD, 0);
					gui_draw_string(VERSION_STRING, 5 + 7 * 8, 64 - 32, FNT_MD, 0);
					gui_draw_string("(c) Liam Braun 2020", 5, 64 - 8, FNT_SM, 0);
					needsRedraw = 0;
				}
				infoScreen_battery();
		}

		_delay_ms(10);
	}
}

void infoScreen_battery() {
	#ifndef console
	// calculate VCC
	ADCSRA |= _BV(ADSC); // start ADC conversion
	while (ADCSRA & _BV(ADIF)); // wait for conversion to complete
	long adc = (ADCL | (ADCH << 8));
	double vcc = 1024 / (0.917 * adc) + 0.301 / 0.917; // more or less accurate
	
	if (adc != prevAdc || (PINB & 1) == 0) {
		double percentage;
		if ((PINB & 1) == 0) { // charge controller PROG
			if (chargingAnimDelay < 50) {
				chargingAnimDelay++;
				goto ifLoopBreak;
			}
			percentage = chargingAnim; // make charging animation
			chargingAnim += 0.2;
			if (chargingAnim > 1) chargingAnim = 0.2;
			gui_draw_string("Battery charging ", 2, 2, FNT_SM, 0);
		} else {
			chargingAnim = 0.2;
			percentage = (vcc - 3.2) / (4.2 - 3.2); // interpolate linearly between 3.2 and 4.2
			sprintf(strBuf, "Battery: %.1fV/%d%%", vcc, (int) (percentage * 100));
			gui_draw_string(strBuf, 2, 2, FNT_SM, 0);
		}
		gui_draw_rect(104, 0, 22, 6, 0);
		gui_draw_line(127, 2, 127, 4);
		for (int i = 0; i < (int) (percentage * 22); i++) {
			gui_draw_line(104 + i, 0, 104 + i, 6);
		}
		gui_clear_rect(104 + (int) (percentage * 22), 1, 22 - ((int) (percentage * 22)), 4);
		chargingAnimDelay = 0;
	}
	ifLoopBreak:
	prevAdc = adc;
	#endif
}

void buttonPressed_calc(int buttonID) {
	if (needsClearRes && buttonID != enter) {
		gui_clear_rect(0, 16, SCREEN_WIDTH, fonts[FNT_MD][3]);
		needsClearRes = 0;
	}

	mathinput_buttonPress(mainScreenInput, buttonID);

	if (buttonID == enter) {
		termTree = parse_term(mainScreenInput->buffer);
		double res = evaluate_term(termTree, 0);
		term_free(termTree);
		char resBuf[16];
		sprintf(resBuf, "%g", res);
		gui_draw_string(resBuf, 0, 16, FNT_MD, 0);
		needsClearRes = 1;
	}

	if (buttonID == f1) {
		currMode = m_info;
		needsRedraw = 1;
	}
	if (buttonID == f2) {
		currMode = m_graph;
		graph_reset_state();
		mathinput_buttonPress(mainScreenInput, enter);
		needsRedraw = 1;
	}
	if (buttonID == f3) {
		currMode = m_mandelbrot;
		mandel_reset_state();
		needsRedraw = 1;
	}
	if (buttonID == f4) {
		currMode = m_solve_menu;
		needsRedraw = 1;
	}
	if (buttonID == back) {
		currMode = m_calc;
	}
}

void buttonPressed(int buttonID) {
	if (buttonID == off) {
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
	switch (currMode) {
		case m_calc:
			buttonPressed_calc(buttonID);
			break;
		case m_graph:
		case m_mandelbrot:
		case m_info:
			if (buttonID == back) {
				currMode = m_calc;
				mathinput_clear(mainScreenInput);
				disp_clear();
				needsRedraw = 1;
			}
			break;
		case m_solve_menu:
			if (buttonID == back) {
				currMode = m_calc;
				disp_clear();
				needsRedraw = 1;
			}
			solver_buttonPress(buttonID);
	}
}

#ifndef console
ISR(PCINT2_vect) {}
#endif