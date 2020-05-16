/*
 * display.c
 *
 * Created: 19.03.2020 14:48:52
 *  Author: liamb
 */ 

#include <avr/io.h>
#include <avr/eeprom.h>

#include "../common/display.h"
#include "../common/eep.h"

unsigned char disp_eeprom_buf[1024];

// format: location in EEPROM, size of entire font, width, height
const int fonts[2][4] = {
	{ EEPROM_FONT_6x7, EEPROM_FONT_6x7_LEN, 6, 7 },
	{ EEPROM_FONT_8x16, EEPROM_FONT_8x16_LEN, 8, 16 }
};

void disp_initialize() {
    unsigned char i;

    DDRB |= DISP_CS | DISP_RESET | DISP_A0 | DISP_SCL | DISP_SI;
	
	// setup SPI
    SPCR |= (_BV(SPE) | _BV(MSTR) | _BV(CPOL) | _BV(CPHA));  // SPI enable, Master, mode 3 (,f/4,  MSB)

    PORTB |= DISP_CS; // chip select inactive high

    PORTB &= ~DISP_RESET; // reset display
    PORTB |= DISP_RESET;

    for (i = 0; i < DISP_INIT_LEN; i++) {
        disp_command(disp_initSequence[i]);
    }
}

void disp_clear() {
    int page, col;
    for (page = 8; page >= 0; page--) {
        disp_command(DISP_CMD_PAGE | page); // set page
        disp_command(DISP_CMD_COL_MSB);
        disp_command(DISP_CMD_COL_LSB);
        for (col = 128; col >= 0; col--) {
            disp_data(0x00);
        }
    }
}

void disp_drawChar(int xPos, int yPos, int num, int font) {
	int fontWidth = fonts[font][2];
	int fontByteWidth = 1;
	int fontHeight = fonts[font][3];
	char tmp_chr[256];
	eep_read_block(&tmp_chr, fonts[font][0] + num * fontByteWidth * fontHeight, fontByteWidth * fontHeight);
	int yEnd = yPos + fontHeight;
	int yStart = yPos;
	while (yPos < yEnd) {
		disp_command(0xB0 | yPos / 8); // set page
		disp_command(0x10 | (xPos >> 4 & 0xF)); // set MSB of column address
		disp_command(0x00 | (xPos      & 0xF)); // set LSB of column address
		for (int col = 1; col <= fontWidth; col++) {
			char colBuf = 0;
			for (int i = 0; i < (((yEnd - yPos) < 8) ? (yEnd - yPos) : 8); i++) {
				int shift = fontWidth - col;
				colBuf += ((tmp_chr[yPos - yStart + i] >> shift) & 1) << i;
			}
			disp_data(colBuf);
		}
		yPos += 8;
	}
}

void disp_drawImage() {
	eep_read_block(&disp_eeprom_buf, EEPROM_STARTUP, EEPROM_STARTUP_LEN);
    for (int page = 7; page >= 0; page--) {
        disp_command(DISP_CMD_PAGE | page); // set page
        disp_command(DISP_CMD_COL_MSB);
        disp_command(DISP_CMD_COL_LSB);
        for (int x = 0; x < 128; x++) {
            char column = 0;
            for (int y = 8; y >= 0; y--) {
				char curr = disp_eeprom_buf[page * 128 + (x / 8) + y * 16];
				//char curr = 0xAA;
                char shift = 7 - (x & 0x7);
                column += ((curr >> shift) & 1) << y;
            }
            disp_data(column);
        }
    }
}

void disp_data(char cmd) {
    PORTB |= DISP_A0;
    disp_sendByte(cmd);
}

void disp_command(char cmd) {
    PORTB &= ~DISP_A0;
    disp_sendByte(cmd);
}

void disp_sendByte(char data) {
    PORTB &= ~DISP_CS;
    SPDR = data;
    while (!(SPSR & (_BV(SPIF)))); // wait for buffer to be ready
    PORTB |= DISP_CS;
}