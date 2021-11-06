#include "../common/display.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
	#include <io.h>
	#include <fcntl.h>
#endif

unsigned char buffer[128*8];
int page = 0;
int xPos = 0;

char stdoutLineBuf[1024*8];

void disp_initialize() {
	#ifdef _WIN32
	setmode(fileno(stdout), O_BINARY);
	setvbuf(stdout, stdoutLineBuf, _IOLBF, 1024*8);
	#endif
}

void disp_clear() {
	memset(buffer, 0, 128 * 8);
}

void disp_data(char cmd) {
	if (realX < 0 || realX > 128 || page < 0 || page > 7) return;
    buffer[page * 128 + realX] = cmd;
	xPos++;
}

void disp_update_data(char cmd) {
    if (realX < 0 || realX > 128 || page < 0 || page > 7) return;
    buffer[page * 128 + realX] |= cmd;
	xPos++;
}

void disp_remove_data(char cmd) {
    if (realX < 0 || realX > 128 || page < 0 || page > 7) return;
    buffer[page * 128 + realX] &= cmd;
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

void get_disp(unsigned char *out) {
	memcpy(out, buffer, 128*8);
}