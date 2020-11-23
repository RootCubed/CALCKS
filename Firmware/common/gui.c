#include "gui.h"

// format: location in EEPROM, size of entire font, width, height
const int fonts[2][4] = {
	{ EEPROM_FONT_6x7, EEPROM_FONT_6x7_LEN, 6, 7 },
	{ EEPROM_FONT_8x16, EEPROM_FONT_8x16_LEN, 8, 16 }
};

void gui_draw_byte(char bt, int x, int y) {
    if (x < 0 || x > 128 || y < 0 || y > 64) return;

    int offset = y % 8;

    disp_setPage(y / 8);
    disp_setXPos(x);
    disp_data((bt << offset) & 0xFF);
    if (offset != 0) {
        disp_setPage(y / 8 + 1);
        disp_setXPos(x);
        unsigned short btUS = bt;
        btUS = btUS & 0x00FF;
        unsigned char res = (char) (btUS >> (8 - offset));
        res &= 0xFF;
        disp_data(0x0A);
    }
}

void gui_update_byte(char bt, int x, int y) {
    if (x < 0 || x > 128 || y < 0 || y > 64) return;

    int offset = y % 8;

    disp_setPage(y / 8);
    disp_setXPos(x);
    disp_update_data((bt << offset) & 0xFF);
    if (y % 8 != 0) {
        disp_setPage(y / 8 + 1);
        disp_setXPos(x);
        disp_update_data(((short) bt & 0x00FF) >> (8 - offset));
    }
}

void gui_remove_byte(char bt, int x, int y) {
    if (x < 0 || x > 128 || y < 0 || y > 64) return;

    int offset = y % 8;

    disp_setPage(y / 8);
    disp_setXPos(x);
    disp_remove_data((bt << offset) & 0xFF | ~((0xFF << offset) & 0xFF));
    if (y % 8 != 0) {
        disp_setPage(y / 8 + 1);
        disp_setXPos(x);
        disp_remove_data(((0xFF00 | bt) >> (8 - offset)) & 0xFF);
    }
}

void gui_draw_char(int xPos, int yPos, int num, int font, int isInverted) {
	int fontWidth = fonts[font][2];
	int fontByteWidth = 1;
	int fontHeight = fonts[font][3];
	char tmp_chr[256];
	eep_read_block(&tmp_chr, fonts[font][0] + num * fontByteWidth * fontHeight, fontByteWidth * fontHeight);
	int yEnd = yPos + fontHeight;
    if (isInverted) yEnd++; // bigger padding
	int yStart = yPos;
	while (yPos < yEnd) {
		for (int col = 0; col < fontWidth; col++) {
			char colBuf = 0;
            int endI = (((yEnd - yPos) < 8) ? (yEnd - yPos) : 8);
			for (int i = 0; i < endI; i++) {
				int shift = fontWidth - col - 1;
                char character = (tmp_chr[yPos - yStart + i]);
                char bitToAdd = ((character >> shift) & 1);
                if (isInverted && yPos + 8 >= yEnd && i + 1 == endI) { // make sure last row is 0
                    bitToAdd = 0;
                }
                if (isInverted) {
                    bitToAdd ^= 1;
                }
                colBuf += bitToAdd << i;
			}
            gui_remove_byte(colBuf, xPos + col, yPos);
			gui_update_byte(colBuf, xPos + col, yPos);
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
                    buf[i] = CHAR_POINT;
                    break;
                case '+':
                    buf[i] = CHAR_PLUS;
                    break;
                case '-':
                    buf[i] = CHAR_MINUS;
                    break;
                case '*':
                    buf[i] = CHAR_MULT;
                    break;
                case '/':
                    buf[i] = CHAR_DIV;
                    break;
                case '=':
                    buf[i] = CHAR_EQUALS;
                    break;
                case '(':
                    buf[i] = CHAR_BROPEN;
                    break;
                case ')':
                    buf[i] = CHAR_BRCLOSE;
                    break;
                case ':':
                    buf[i] = CHAR_COLON;
                    break;
                case '^':
                    buf[i] = CHAR_POW;
                    break;
                case '%':
                    buf[i] = CHAR_PERCENT;
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
            gui_clear_rect(xPos + (i - 1) * fonts[font][2], yPos, fonts[font][2], fonts[font][3]);
        }
        i++;
    }
    return (i - 1) * fonts[font][2];
}

void gui_draw_image(char buffer[1024]) {
    for (int page = 7; page >= 0; page--) {
        disp_setPage(page);
        disp_setXPos(0);
        for (int x = 0; x < 128; x++) {
            char column = 0;
            for (int y = 8; y >= 0; y--) {
				char curr = buffer[page * 128 + (x / 8) + y * 16];
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
    if (x1 == x2) {
        if (y1 > y2) {
            int tmp = y1;
            y1 = y2;
            y2 = tmp;
        }
        for (int i = y1; i <= y2; i += 8) {
            char byteToAdd = 0;
            for (int j = i; j <= MIN(i + 8, y2); j++) {
                byteToAdd |= (1 << (j - i));
            }
            gui_update_byte(byteToAdd, x1, i);
        }
        return;
    }
    if (x1 > x2) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    int diffX = x2 - x1;
    int diffY = y2 - y1;
    double q = (double) diffY / diffX;
    for (double cX = x1; cX <= x2; cX += 0.1) {
        if (round(cX) < 0 || round(cX) > 128 || y1 + round(q * (cX - x1)) < 0 || y1 + round(q * (cX - x1)) > 64) continue;
        gui_update_byte(0x01, round(cX), y1 + round(q * (cX - x1)));
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
            for (int j = i; j <= MIN(i + 8, y2); j++) {
                byteToRemove ^= (1 << (j - i));
            }
            gui_remove_byte(byteToRemove, x1, i);
        }
        return;
    }
    double q = (double) diffY / diffX;
    for (int cX = x1; cX <= x2; cX++) {
        gui_remove_byte(0xFE, cX, y1 + round(q * (cX - x1)));
    }
}

void gui_draw_rect(int x, int y, int width, int height, int isFilled) {
    if (width < 1 || height < 1 || x < 0 || y < 0) return;
    width -= 1;
    height -= 1;
    gui_draw_line(x, y, x, y + height);
    gui_draw_line(x, y, x + width, y);
    gui_draw_line(x, y + height, x + width, y + height);
    gui_draw_line(x + width, y, x + width, y + height);
    if (isFilled) {
        int endX = MIN(SCREEN_WIDTH, x + width);
        int endY = MIN(SCREEN_HEIGHT, y + height);
        for (int cX = x + 1; cX < endX; cX++) {
            gui_draw_line(cX, y + 1, cX, endY);
        }
    }
}

// circle drawing algorithm from https://web.engr.oregonstate.edu/~sllu/bcircle.pdf

void plot8CirclePoints(int centerX, int centerY, int x, int y) {
    gui_set_pixel(centerX + x, centerY + y * PIXEL_W_H_RATIO, 1);
    gui_set_pixel(centerX - x, centerY + y * PIXEL_W_H_RATIO, 1);
    gui_set_pixel(centerX - x, centerY - y * PIXEL_W_H_RATIO, 1);
    gui_set_pixel(centerX + x, centerY - y * PIXEL_W_H_RATIO, 1);
    gui_set_pixel(centerX + y, centerY + x * PIXEL_W_H_RATIO, 1);
    gui_set_pixel(centerX - y, centerY + x * PIXEL_W_H_RATIO, 1);
    gui_set_pixel(centerX - y, centerY - x * PIXEL_W_H_RATIO, 1);
    gui_set_pixel(centerX + y, centerY - x * PIXEL_W_H_RATIO, 1);
}

void plot8CircleMidLines(int centerX, int centerY, int x, int y) {
    gui_draw_line(centerX, centerY, centerX + x, centerY + y * PIXEL_W_H_RATIO);
    gui_draw_line(centerX, centerY, centerX - x, centerY + y * PIXEL_W_H_RATIO);
    gui_draw_line(centerX, centerY, centerX - x, centerY - y * PIXEL_W_H_RATIO);
    gui_draw_line(centerX, centerY, centerX + x, centerY - y * PIXEL_W_H_RATIO);
    gui_draw_line(centerX, centerY, centerX + y, centerY + x * PIXEL_W_H_RATIO);
    gui_draw_line(centerX, centerY, centerX - y, centerY + x * PIXEL_W_H_RATIO);
    gui_draw_line(centerX, centerY, centerX - y, centerY - x * PIXEL_W_H_RATIO);
    gui_draw_line(centerX, centerY, centerX + y, centerY - x * PIXEL_W_H_RATIO);
}

void gui_draw_circle(int x, int y, int radius, int isFilled) {
    if (radius < 1 || x < 0 || y < 0) return;
    int xChange = 1 - 2 * radius;
    int yChange = 1;
    int currX = radius;
    int currY = 0;
    int radiusError = 0;
    while (currX >= currY) {
        plot8CirclePoints(x, y, currX, currY);
        if (isFilled) {
            plot8CircleMidLines(x, y, currX, currY);
        }
        currY++;
        radiusError += yChange;
        yChange += 2;
        if (2 * radiusError + xChange > 0) {
            currX--;
            radiusError += xChange;
            xChange += 2;
        }
    }
}

void gui_clear_rect(int x, int y, int width, int height) {
    width -= 1;
    height -= 1;
    int endX = MIN(SCREEN_WIDTH, x + width);
    int endY = MIN(SCREEN_HEIGHT, y + height);
    for (int cX = x; cX <= endX; cX++) {
        gui_clear_line(cX, y, cX, endY);
    }
}

int gui_tab_button(const char *tabName, int xPos) {
    gui_draw_byte(0b11111110, xPos, 56);
    int length = gui_draw_string(tabName, xPos + 1, 56, FNT_SM, 1);
    gui_draw_byte(0b11111110, xPos + 1 + length, 56);
    return length + 2;
}