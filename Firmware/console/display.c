#include "../common/display.h"
#include <stdio.h>
#include <string.h>
#include <io.h>

// overridden

char buffer[128*8];
int page = 0;
int xPos = 0;

void disp_initialize() {
	freopen(NULL, "wb", stdout);
	setbuf(stdout, NULL);
}

void disp_clear() {
	memset(buffer, 0, 128 * 8);
}

void disp_data(char cmd) {
	if (xPos > 128 || page > 7) return;
    buffer[page * 128 + xPos] = cmd;
	xPos++;
}

void disp_command(char cmd) {
    if ((cmd & 0xF0) == DISP_CMD_PAGE) {
		page = cmd & 0xF;
	}
	if ((cmd & 0xF0) == DISP_CMD_COL_LSB) {
		xPos &= 0b11110000;
		xPos |= (cmd & 0x0F);
	}
	if ((cmd & 0xF0) == DISP_CMD_COL_MSB) {
		xPos &= 0b00001111;
		xPos |= (cmd & 0x0F) << 4;
	}
}

void disp_print() {
	fwrite(buffer, 1, 128 * 8, stdout);
}

// original

unsigned char disp_eeprom_buf[1024];

// format: location in EEPROM, size of entire font, width, height
const int fonts[2][4] = {
	{ EEPROM_FONT_6x7, EEPROM_FONT_6x7_LEN, 6, 7 },
	{ EEPROM_FONT_8x16, EEPROM_FONT_8x16_LEN, 8, 16 }
};

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
	eep_read_block(&disp_eeprom_buf, EEPROM_STARTUP, 1024);
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