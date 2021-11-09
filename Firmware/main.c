#define F_CPU 8000000L

#define VERSION_STRING "v1.0.0"

#include "common/display.h"
#include "common/buttons.h"
#include "common/eep.h"
#include "common/gui.h"
#include "common/term.h"
#include "common/apps.h"
#include "common/mandel.h"
#include "common/graph.h"
#include "common/mathinput.h"
#include "common/solver.h"
#include "common/uart.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef console

void press_button(unsigned char btn, int special) {
	if (special) {
		set_special_button(btn);
	} else {
		set_button(btn);
	}
}

void get_screen(unsigned char *buf) {
	get_disp(buf);
}

#else
	#include <avr/io.h>
	#include <util/delay.h>
	#include <avr/sleep.h>
	#include <avr/interrupt.h>
	#include <avr/wdt.h>
#endif

#define BTN_UP 0b00001000
#define BTN_LEFT 0b00010000
#define BTN_DOWN 0b01000000
#define BTN_RIGHT 0b00100000

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

long noInputTimeout = 0;

int screenBeforeError;

int info_mode = 0;

void buttonPressed(int);
void infoScreen_battery();

void errorScreen(char *);

void screen_tick();

#ifndef console
// https://stackoverflow.com/questions/32802221/how-to-write-a-custom-reset-function-in-c-for-avr-studio

// Function Prototype
/*void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));


// Function Implementation
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();

    return;
}*/

int freeRam() {
	int *v = malloc(1);
	free(v);
	return (int) v;
}
#endif

int main(void) {
	buttons_initialize();
	disp_initialize();
	eep_initialize();
	
	unsigned char disp_eeprom_buf[1024];
	eep_read_block(&disp_eeprom_buf, EEPROM_STARTUP, EEPROM_STARTUP_LEN);
	gui_draw_image(disp_eeprom_buf);
	
	buttons_initialize();

	mainScreenInput = mathinput_initBox(FNT_MD, 100, 0, 0);
	term_init();
	solver_init();
	graph_init();

	#ifndef console
	// init ADC
	ADMUX = (0x01 << REFS0) /* AVCC with external capacitor at AREF pin */
			| (0 << ADLAR) /* Left Adjust Result: disabled */
			| (0b11110 << MUX0) /* 1.1V (Vbg) */;
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= (0b100 << ADPS0); // prescaler 16

    uart_init();
	#endif
	
	noInputTimeout = 0;

	#ifndef console

	_delay_ms(2000);
	disp_clear();

	while (1) {
		screenTick();

		_delay_ms(10);
		noInputTimeout += 10;
		if (noInputTimeout > (long) 1000 * 60 * 2) buttonPressed(off);
	}
	#endif
}

void screen_tick() {
char specialButtons = buttons_get_special();
	if ((lastSpecialButtonState & BTN_UP) && !(specialButtons & BTN_UP)) {
		noInputTimeout = 0;
		buttonPressed(btn_up);
	}
	if ((lastSpecialButtonState & BTN_DOWN) && !(specialButtons & BTN_DOWN)) {
		noInputTimeout = 0;
		buttonPressed(btn_down);
	}
	if ((lastSpecialButtonState & BTN_LEFT) && !(specialButtons & BTN_LEFT)) {
		noInputTimeout = 0;
		buttonPressed(btn_left);
	}
	if ((lastSpecialButtonState & BTN_RIGHT) && !(specialButtons & BTN_RIGHT)) {
		noInputTimeout = 0;
		buttonPressed(btn_right);
	}
	lastSpecialButtonState = specialButtons;
	int btnUnmapped = buttons_getPressed();
	int currButton = -1;
	if (btnUnmapped > -1) {
		noInputTimeout = 0;
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
			graph_updateScreen();
			break;
		case m_solve_menu:
			solver_updateScreen();
			break;
		case m_applist:
			applist_updateScreen();
			break;
		case m_info:
			if (needsRedraw) {
				disp_clear();
				if (info_mode == 0) {
					prevAdc = 0;
					chargingAnimDelay = 50;
					gui_draw_string("CALCKS", 5, 64 - 44, FNT_MD, 0);
					gui_draw_string(VERSION_STRING, 5 + 7 * 8, 64 - 44, FNT_MD, 0);
					gui_draw_string("(c) Liam Braun 2021", 5, 64 - 20, FNT_SM, 0);

					gui_tab_button("About", 0);
					gui_tab_button("More", 100);
				} else if (info_mode == 1) {
					const char *strings[3] = {
						"CALCKS",
						"Das Maturprojekt von",
						"Liam Braun"
					};
					for (int i = 0; i < sizeof(strings) / sizeof(char *); i++) {
						gui_draw_string(strings[i], 0, i * 8, FNT_SM, 0);
					}
				} else if (info_mode == 2) {
					#ifndef console
					extern unsigned int __data_start;
					extern unsigned int __data_end;
					extern unsigned int __bss_start;
					extern unsigned int __bss_end;
					extern unsigned int __heap_start;

					u8 stackPointerL = *(u8*)0x005d;
					u8 stackPointerH = *(u8*)0x005e;
					sprintf(strBuf, ".data");
					gui_draw_string(strBuf, 0, 0, FNT_SM, 0);
					sprintf(strBuf, "%d-%d", (int) &__data_start, (int) & __data_end);
					gui_draw_string(strBuf, 0, 8, FNT_SM, 0);

					sprintf(strBuf, ".bss");
					gui_draw_string(strBuf, 0, 18, FNT_SM, 0);
					sprintf(strBuf, "%d-%d", (int) &__bss_start, (int) &__bss_end);
					gui_draw_string(strBuf, 0, 26, FNT_SM, 0);

					sprintf(strBuf, "Heap");
					gui_draw_string(strBuf, 0, 36, FNT_SM, 0);
					sprintf(strBuf, "%d-%d", (int) &__heap_start, freeRam());
					gui_draw_string(strBuf, 0, 44, FNT_SM, 0);
					
					sprintf(strBuf, "SP = %d", (stackPointerH << 8) + stackPointerL);
					gui_draw_string(strBuf, 0, 56, FNT_SM, 0);
					#endif
				}
				needsRedraw = 0;
			}
			if (info_mode == 0) {
				infoScreen_battery();
			}
	}
}

void errorScreen(char *errorText) {
	screenBeforeError = currMode;
	disp_clear();
	gui_draw_string(errorText, (SCREEN_WIDTH - 13 * fonts[FNT_MD][2]) / 2, 24, FNT_MD, 0);
	gui_draw_string("OK", (SCREEN_WIDTH - 3 * fonts[FNT_SM][2]) / 2, 50, FNT_SM, 1);
	currMode = m_error;
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
		if (chargingAnimDelay < 5) {
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
			errorScreen("Syntax Error");
			return;
		}
		termTree = term_parse(mainScreenInput->buffer);
		double res = term_evaluate(termTree, 0);
		term_free(termTree, 1);
		char resBuf[16];
		sprintf(resBuf, "%.10g", res);
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
		//mathinput_buttonPress(mainScreenInput, enter);
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

		noInputTimeout = 0;
	}
	if (buttonID == btn_apps) {
		currMode = m_applist;
		disp_clear();
		return;
	}
	switch (currMode) {
		case m_calc:
			buttonPressed_calc(buttonID);
			break;
		case m_graph:
			if (graph_buttonPress(buttonID) != 0) {
				errorScreen("Syntax Error");
			}
		case m_mandelbrot:
			if (buttonID == back) {
				currMode = m_calc;
				disp_clear();
				needsRedraw = 1;
			}
			break;
		case m_applist:
			if (buttonID == back) {
				currMode = m_calc;
				disp_clear();
				needsRedraw = 1;
			}
			applist_buttonPress(buttonID, &currMode);
			break;
		case m_info:
			if (buttonID == back) {
				if (info_mode == 0) {
					currMode = m_calc;
					disp_clear();
				} else {
					info_mode = 0;
				}
				needsRedraw = 1;
			}
			if (info_mode == 0) {
				if (buttonID == bracket_open) {
					disp_clear();
					gui_draw_string("     Hello      ", 0, 64 - 48, FNT_MD, 0);
					gui_draw_string("   @Brammyson   ", 0, 64 - 32, FNT_MD, 0);
					gui_draw_string("   @Kaoskarl    ", 0, 64 - 16, FNT_MD, 0);
				}
				if (buttonID == f1) {
					info_mode = 1;
					needsRedraw = 1;
				}
				if (buttonID == f4) {
					info_mode = 2;
					needsRedraw = 1;
				}
			}
			break;
		case m_solve_menu:
			if (buttonID == back) {
				currMode = m_calc;
				disp_clear();
				needsRedraw = 1;
			}
			if (solver_buttonPress(buttonID) != 0) {
				errorScreen("Syntax Error");
			}
			break;
		case m_error:
			if (buttonID == enter) {
				currMode = screenBeforeError;
				needsRedraw = 1;
			}
			break;
	}
}

#ifndef console
ISR(PCINT2_vect) {}
#endif