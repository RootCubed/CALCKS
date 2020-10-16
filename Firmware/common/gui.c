#include "gui.h"

unsigned char disp_eeprom_buf[1024];

// format: location in EEPROM, size of entire font, width, height
const int fonts[2][4] = {
	{ EEPROM_FONT_6x7, EEPROM_FONT_6x7_LEN, 6, 7 },
	{ EEPROM_FONT_8x16, EEPROM_FONT_8x16_LEN, 8, 16 }
};

void gui_draw_byte(char bt, int x, int y) {
    if (x < 0 || x > 128 || y < 0 || y > 64) return;
    x += 4;

    int offset = y % 8;

    disp_setPage(y / 8);
    disp_setMSBCol((x >> 4 & 0xF));
    disp_setLSBCol((x      & 0xF));
    disp_data((bt << offset) & 0xFF);
    if (offset != 0) {
        disp_setPage(y / 8 + 1);
        disp_setMSBCol((x >> 4 & 0xF));
        disp_setLSBCol((x      & 0xF));
        disp_data(((short) bt & 0x00FF) >> (8 - offset));
    }
}

void gui_update_byte(char bt, int x, int y) {
    if (x < 0 || x > 128 || y < 0 || y > 64) return;
    x += 4;

    int offset = y % 8;

    disp_setPage(y / 8);
    disp_setMSBCol((x >> 4 & 0xF));
    disp_setLSBCol((x      & 0xF));
    disp_update_data((bt << offset) & 0xFF);
    if (y % 8 != 0) {
        disp_setPage(y / 8 + 1);
        disp_setMSBCol((x >> 4 & 0xF));
        disp_setLSBCol((x      & 0xF));
        disp_update_data(((short) bt & 0x00FF) >> (8 - offset));
    }
}

void gui_remove_byte(char bt, int x, int y) {
    if (x < 0 || x > 128 || y < 0 || y > 64) return;
    x += 4;

    int offset = y % 8;

    disp_setPage(y / 8);
    disp_setMSBCol((x >> 4 & 0xF));
    disp_setLSBCol((x      & 0xF));
    disp_remove_data((bt << offset) & 0xFF | ~((0xFF << offset) & 0xFF));
    if (y % 8 != 0) {
        disp_setPage(y / 8 + 1);
        disp_setMSBCol((x >> 4 & 0xF));
        disp_setLSBCol((x      & 0xF));
        disp_remove_data(((0xFF00 | bt) >> (8 - offset)) & 0xFF);
    }
}

void gui_draw_char(int xPos, int yPos, int num, int font, int isInverted) {
    xPos += 4;
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

void convert_string(const char *str, char *buf) {
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
                case '.':
                    buf[i] = 71;
                    break;
                case '+':
                    buf[i] = 62;
                    break;
                case '-':
                    buf[i] = 63;
                    break;
                case '*':
                    buf[i] = 64;
                    break;
                case '/':
                    buf[i] = 65;
                    break;
                case '=':
                    buf[i] = 66;
                    break;
                case '(':
                    buf[i] = 68;
                    break;
                case ')':
                    buf[i] = 69;
                    break;
                case ':':
                    buf[i] = 72;
                    break;
                case '^':
                    buf[i] = 73;
                    break;
                case '%':
                    buf[i] = 74;
                    break;
            }
        }
        i++;
    }
    *(buf - 1) = i + 1;
}

int gui_draw_string(const char *str, int xPos, int yPos, int font, int isInverted) {
    convert_string(str, realStringBuf);
    int len = realStringBuf[0];
    int i = 1;
    while (i < len) {
        if (realStringBuf[i] != -1) {
            gui_draw_char(xPos + (i - 1) * fonts[font][2], yPos, realStringBuf[i], font, isInverted);
        } else {
            gui_clear_rect(xPos + (i - 1) * fonts[font][2], yPos, fonts[font][2], fonts[font][3] - 1);
        }
        i++;
    }
    return (i - 1) * fonts[font][2];
}

void gui_draw_image() {
	eep_read_block(&disp_eeprom_buf, EEPROM_STARTUP, EEPROM_STARTUP_LEN);
    for (int page = 7; page >= 0; page--) {
        disp_setPage(page);
        disp_setMSBCol(0);
        disp_setLSBCol(4);
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

void gui_set_pixel(int x, int y, int value) {
    value = value ? 1 : 0;
    gui_update_byte(value, x, y);
}

void gui_draw_line(int x1, int y1, int x2, int y2) {
    if (x1 > x2) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    int diffX = x2 - x1;
    int diffY = y2 - y1;
    if (diffX == 0) {
        for (int i = y1; i <= y2; i += 8) {
            char byteToRemove = 0;
            for (int j = i; j < MIN(i + 8, y2); j++) {
                byteToRemove |= (1 << (j - i));
            }
            gui_update_byte(byteToRemove, x1, i);
        }
        return;
    }
    double q = (double) diffY / diffX;
    for (int cX = x1; cX <= x2; cX++) {
        gui_update_byte(0x01, cX, y1 + ceil(q * (cX - x1)));
    }
}

void gui_clear_line(int x1, int y1, int x2, int y2) {
    if (x1 > x2) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    int diffX = x2 - x1;
    int diffY = y2 - y1;
    if (diffX == 0) {
        for (int i = y1; i <= y2; i += 8) {
            char byteToRemove = 0xFF;
            for (int j = i; j < MIN(i + 8, y2); j++) {
                byteToRemove ^= (1 << (j - i));
            }
            gui_remove_byte(byteToRemove, x1, i);
        }
        return;
    }
    double q = (double) diffY / diffX;
    for (int cX = x1; cX <= x2; cX++) {
        gui_remove_byte(0xFE, cX, y1 + ceil(q * (cX - x1)));
    }
}

void gui_draw_rect(int x, int y, int width, int height, int isFilled) {
    if (width < 0 || height < 0 || x < 0 || y < 0) return;
    gui_draw_line(x, y, x, y + height);
    gui_draw_line(x, y, x + width, y);
    gui_draw_line(x, y + height, x + width, y + height);
    gui_draw_line(x + width, y, x + width, y + height);
    if (isFilled) {
        int endX = MIN(SCREEN_WIDTH, x + width);
        int endY = MIN(SCREEN_HEIGHT, y + height);
        for (int cX = x; cX < endX; cX++) {
            gui_draw_line(cX, y, cX, endY);
        }
    }
}

void gui_draw_circle(int x, int y, int dia, int isFilled) {
    if (dia < 0 || x < 0 || y < 0) return;
    for (int i = 0; i < 360; i++) {
        double rads = i * 3.1415926/180;
        gui_update_byte(0x01, round(dia * sin(rads)) + x, round(dia * cos(rads) * 0.85714) + y);
    }
}

void gui_clear_rect(int x, int y, int width, int height) {
    int endX = MIN(SCREEN_WIDTH, x + width);
    int endY = MIN(SCREEN_HEIGHT, y + height);
    for (int cX = x; cX < endX; cX++) {
        gui_clear_line(cX, y, cX, endY);
    }
}

int gui_tab_button(const char *tabName, int xPos) {
    gui_draw_byte(0b11111110, xPos, 56);
    int length = gui_draw_string(tabName, xPos + 1, 57, FNT_SM, 1);
    gui_draw_byte(0b11111110, xPos + 1 + length, 56);
    return length + 2;
}