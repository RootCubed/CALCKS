#define F_CPU 8000000L

#define VERSION_STRING "v0.9.2"

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
			if (code == 's') {
				char button;
				scanf("%d", &button);
				set_special_button(button);
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

#define BTN_UP 0b00001000
#define BTN_LEFT 0b00010000
#define BTN_DOWN 0b01000000
#define BTN_RIGHT 0b00100000

enum modes {
	m_calc,
	m_mandelbrot,
	m_graph,
	m_solve_menu,
	m_info,
	m_error
};

char resBuf[0x20];
int lastButton = -1;
int lastSpecialButtonState = 0xFFFF;
opNode *termTree;

char strBuf[64];

inputBox *mainScreenInput;
int needsClearRes = 0;

int currMode = m_calc;
int needsRedraw = 1;

int prevAdc = 0;
float chargingAnim = 0.2;
int chargingAnimDelay = 50;

void buttonPressed(int);
void infoScreen_battery();

#ifndef console
// https://jeelabs.org/2011/05/22/atmega-memory-use/
int freeRam() {
	extern int __heap_start, *__brkval; 
	int v; 
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#endif

int main(void) {
	#ifdef console
		CreateThread(NULL, 0, ConsoleListener, NULL, 0, NULL);
	#endif
	buttons_initialize();
	disp_initialize();
	eep_initialize();
	
	unsigned char disp_eeprom_buf[1024];
	eep_read_block(&disp_eeprom_buf, EEPROM_STARTUP, EEPROM_STARTUP_LEN);
	gui_draw_image(disp_eeprom_buf);
	
	_delay_ms(2000);
	
	disp_clear();
	buttons_initialize();

	mainScreenInput = mathinput_initBox(FNT_MD, 100, 0, 0);
	solver_init();
	graph_init();

	#ifndef console
	// init ADC
	ADMUX = (0x01 << REFS0) /* AVCC with external capacitor at AREF pin */
			| (0 << ADLAR) /* Left Adjust Result: disabled */
			| (0b11110 << MUX0) /* 1.1V (Vbg) */;
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= (0b100 << ADPS0); // prescaler 16

	#endif

	while (1) {
		char specialButtons = buttons_get_special();
		if ((lastSpecialButtonState & BTN_UP) && !(specialButtons & BTN_UP)) {
			buttonPressed(up);
		}
		if ((lastSpecialButtonState & BTN_DOWN) && !(specialButtons & BTN_DOWN)) {
			buttonPressed(down);
		}
		if ((lastSpecialButtonState & BTN_LEFT) && !(specialButtons & BTN_LEFT)) {
			buttonPressed(left);
		}
		if ((lastSpecialButtonState & BTN_RIGHT) && !(specialButtons & BTN_RIGHT)) {
			buttonPressed(right);
		}
		lastSpecialButtonState = specialButtons;
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
					disp_clear();
					gui_tab_button("Info", 0);
					gui_tab_button("Graph", 28);
					gui_tab_button("Mandel", 61);
					gui_tab_button("Solv", 100);
					mathinput_redraw(mainScreenInput);
					mathinput_setCursor(mainScreenInput, CURSOR_ON);
					// gui test
					/*gui_set_pixel(0, 0, 1);
					gui_set_pixel(1, 0, 1);
					gui_set_pixel(0, 1, 1);
					gui_draw_line(1, 1, 4, 4);
					gui_draw_line(5, 0, 5, 4);
					gui_draw_rect(0, 5, 2, 3, 0);
					gui_draw_rect(0, 10, 2, 3, 1);
					gui_draw_rect(0, 15, 4, 4, 0);
					gui_draw_rect(0, 20, 4, 4, 1);
					gui_draw_rect(0, 25, 6, 6, 1);
					gui_clear_rect(1, 26, 4, 4);
					gui_draw_rect(0, 34, 20, 6, 1);
					gui_draw_circle(50, 20, 10, 1);
					gui_clear_rect(45, 21, 4, 3);
					gui_draw_char(50, 23, 7, FNT_SM, 1);
					gui_clear_line(50, 15, 55, 16);
					gui_draw_circle(80, 20, 10, 0);
					gui_update_byte(0xAA, 29, 8);
					gui_update_byte(0xAA, 30, 0);
					gui_update_byte(0xAA, 31, 1);
					gui_update_byte(0xAA, 32, 2);
					gui_update_byte(0xAA, 33, 3);
					gui_update_byte(0xAA, 34, 4);
					gui_update_byte(0xAA, 35, 5);
					gui_update_byte(0xAA, 36, 6);
					gui_update_byte(0xAA, 37, 7);
					gui_update_byte(0xAA, 38, 8);
					gui_draw_char(60, 38, 7, FNT_SM, 0);
					gui_draw_char(70, 39, 7, FNT_SM, 0);
					gui_draw_char(80, 40, 7, FNT_SM, 0);
					gui_draw_char(90, 41, 7, FNT_SM, 0);
					gui_draw_char(100, 42, 7, FNT_SM, 0);
					gui_draw_char(110, 30, CHAR_MULT, FNT_MD, 0);*/
				}
				needsRedraw = 0;
				mathinput_cursorFrame(mainScreenInput);
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
				#ifndef console
				sprintf(strBuf, "RAM Usage:");
				gui_draw_string(strBuf, 0, 12, FNT_SM, 0);
				sprintf(strBuf, "%d/16384 B", 16384 - freeRam());
				gui_draw_string(strBuf, 0, 20, FNT_SM, 0);
				#endif
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
		if (adc == prevAdc) goto ifLoopBreak;
		chargingAnim = 0.2;
		percentage = (vcc - 3.2) / (4.2 - 3.2); // interpolate linearly between 3.2 and 4.2
		sprintf(strBuf, "Battery:%.1fV/%3d%%", vcc, (int) (percentage * 100));
		gui_draw_string(strBuf, 2, 2, FNT_SM, 0);
	}
	gui_draw_rect(104, 2, 23, 6, 0);
	gui_draw_line(127, 3, 127, 6);
	for (int i = 0; i < (int) (percentage * 22); i++) {
		gui_draw_line(104 + i, 3, 104 + i, 6);
	}
	gui_clear_rect(104 + (int) (percentage * 22), 3, 22 - ((int) (percentage * 22)), 4);
	chargingAnimDelay = 0;
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
		if (mathinput_checkSyntax(mainScreenInput)) {
			disp_clear();
			gui_draw_string("Syntax error", (SCREEN_WIDTH - 13 * fonts[FNT_MD][2]) / 2, 24, FNT_MD, 0);
			gui_draw_string("OK", (SCREEN_WIDTH - 3 * fonts[FNT_SM][2]) / 2, 50, FNT_SM, 1);
			currMode = m_error;
			return;
		}
		termTree = parse_term(mainScreenInput->buffer);
		double res = evaluate_term(termTree, 0);
		term_free(termTree);
		char resBuf[16];
		sprintf(resBuf, "%g", res);
		gui_draw_string(resBuf, 0, 16, FNT_MD, 0);
		needsClearRes = 1;
		mathinput_setCursor(mainScreenInput, CURSOR_HIDDEN);
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
			graph_buttonPress(buttonID);
		case m_mandelbrot:
		case m_info:
			if (buttonID == back) {
				currMode = m_calc;
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
			break;
		case m_error:
			if (buttonID == enter) {
				currMode = m_calc;
				needsRedraw = 1;
			}
			break;
	}
	if (currMode == m_info) {
		if (buttonID == bracket_open) {
			disp_clear();
			gui_draw_string("     Hello      ", 0, 64 - 48, FNT_MD, 0);
			gui_draw_string("   @Brammyson   ", 0, 64 - 32, FNT_MD, 0);
			gui_draw_string("   @Kaoskarl    ", 0, 64 - 16, FNT_MD, 0);
		}
	}
}

#ifndef console
ISR(PCINT2_vect) {}
#endif