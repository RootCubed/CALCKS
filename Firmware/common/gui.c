#include "gui.h"
#include "eep.h"
#include "display.h"

unsigned char disp_eeprom_buf[1024];

// format: location in EEPROM, size of entire font, width, height
const int fonts[2][4] = {
	{ EEPROM_FONT_6x7, EEPROM_FONT_6x7_LEN, 6, 7 },
	{ EEPROM_FONT_8x16, EEPROM_FONT_8x16_LEN, 8, 16 }
};

void gui_drawByte(char bt, int x, int y) {
    disp_setPage(y / 8);
    disp_setMSBCol((x >> 4 & 0xF));
    disp_setLSBCol((x      & 0xF));
    disp_data(bt);
}

void gui_drawChar(int xPos, int yPos, int num, int font, int isInverted) {
	int fontWidth = fonts[font][2];
	int fontByteWidth = 1;
	int fontHeight = fonts[font][3];
	char tmp_chr[256];
	eep_read_block(&tmp_chr, fonts[font][0] + num * fontByteWidth * fontHeight, fontByteWidth * fontHeight);
	int yEnd = yPos + fontHeight;
	int yStart = yPos;
	while (yPos < yEnd) {
		disp_setPage(yPos / 8);
		disp_setMSBCol((xPos >> 4 & 0xF));
		disp_setLSBCol((xPos      & 0xF));
		for (int col = 1; col <= fontWidth; col++) {
			char colBuf = 0;
			for (int i = 0; i < (((yEnd - yPos) < 8) ? (yEnd - yPos) : 8); i++) {
				int shift = fontWidth - col;
                char character = (tmp_chr[yPos - yStart + i]);
                colBuf += ((character >> shift) & 1) << i;
			}
            if (isInverted) colBuf = ~colBuf;
			disp_data(colBuf);
		}
		yPos += 8;
	}
}

char realStringBuf[64];

void convertString(const char *str, char *buf) {
    buf = buf + 1; // start one later
    int i = 0;
    while (str[i] != 0) {
        if (str[i] >= '0' && str[i] <= '9') {
            buf[i] = str[i] - '0';
        } else if (str[i] >= 'a' && str[i] <= 'z') {
            buf[i] = str[i] - 'a' + 36;
        } else if (str[i] >= 'A' && str[i] <= 'Z') {
            buf[i] = str[i] - 'A' + 10;
        } else {
            switch (str[i]) {
                case ' ':
                    buf[i] = -1;
                    break;
                case '-':
                    buf[i] = 70;
                    break;
                case '.':
                    buf[i] = 71;
                    break;
            }
        }
        i++;
    }
    *(buf - 1) = i + 1;
}

int gui_drawString(const char *str, int xPos, int yPos, int font, int isInverted) {
    convertString(str, realStringBuf);
    int len = realStringBuf[0];
    int i = 1;
    while (i < len) {
        if (realStringBuf[i] != -1) {
            gui_drawChar(xPos + (i - 1) * fonts[font][2], yPos, realStringBuf[i], font, isInverted);
        }
        i++;
    }
    return (i - 1) * fonts[font][2];
}

void gui_drawImage() {
	eep_read_block(&disp_eeprom_buf, EEPROM_STARTUP, EEPROM_STARTUP_LEN);
    for (int page = 7; page >= 0; page--) {
        disp_setPage(page);
        disp_setMSBCol(0);
        disp_setLSBCol(0);
        for (int x = 0; x < 128; x++) {
            char column = 0;
            for (int y = 8; y >= 0; y--) {
				char curr = disp_eeprom_buf[page * 128 + (x / 8) + y * 16];
                char shift = 7 - (x & 0x7);
                column += ((curr >> shift) & 1) << y;
            }
            disp_data(column);
        }
    }
}

int gui_tabButton(const char *tabName, int xPos) {
    gui_drawByte(0b11111110, xPos, 56);
    int length = gui_drawString(tabName, xPos + 1, 57, FNT_SM, 1);
    gui_drawByte(0b11111110, xPos + 1 + length, 56);
    return length + 2;
}