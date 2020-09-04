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
	#include <avr/sleep.h>
	#include <avr/interrupt.h>
	//#include "avr/bmp3.h"
#endif

int cursorX = 0, cursorY = 0;
char currLine[0x20] = "";
char resBuf[0x20];
int lastButton = -1;
u8 currTerm[0x20];

// bmp388
//struct bmp3_dev* bmp;

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

	// BMP388
	/*bmp->intf = BMP3_I2C_INTF;
	int8_t bmp3result = bmp3_init(bmp);
	if (bmp3result >= 0) { // no error
		bmp3_soft_reset(bmp);
		bmp3_set_sensor_settings(BMP3_SEL_PRESS_EN | BMP3_SEL_TEMP_EN, bmp);
	}*/
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
		double vcc = 1024 / (0.917 * adc) + 0.301 / 0.917; // more or less accurate
		char strBuf[16];
		sprintf(strBuf, "%.2fV", vcc);
		gui_draw_string(strBuf, 128 - 5 * 6, 0, FNT_SM, 0);

		// bmp388
		/*if (bmp3result >= 0) {
			struct bmp3_data sensorData;
			bmp3_get_sensor_data(BMP3_ALL, &sensorData, bmp);
			sprintf(strBuf, "%.2f °C", sensorData.temperature);
			gui_draw_string(strBuf, 0, 40, FNT_SM, 0);
		}*/
		#endif
		_delay_ms(20);
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
		opNode* termTree = parse_term(currTerm);
		double res = evaluate_term(termTree);
		term_free(termTree);
		char resBuf[16];
		sprintf(resBuf, "%.3f", res);
		gui_draw_string(resBuf, 0, 16, FNT_MD, 0);
	}
	if (buttonID == 15) {
		// put display into sleep mode (turn display off, turn all points on)
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
	}
}

ISR(PCINT2_vect) {}