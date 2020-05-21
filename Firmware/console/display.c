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