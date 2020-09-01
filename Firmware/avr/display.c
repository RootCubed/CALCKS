#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>

#include "../common/display.h"

void disp_initialize() {
    DDRB |= DISP_CS | DISP_RESET | DISP_A0 | DISP_SCL | DISP_SI;

    PORTB &= ~DISP_SCL;
    PORTB &= ~DISP_CS; // chip select active low

    PORTB &= ~DISP_RESET; // reset display
    PORTB |= DISP_RESET;

    for (int i = 0; i < DISP_INIT_LEN; i++) {
        disp_command(disp_initSequence[i]);
    }
}

void disp_clear() {
    int page, col;
    for (page = 8; page >= 0; page--) {
        disp_command(DISP_CMD_PAGE | page); // set page
        disp_command(DISP_CMD_COL_MSB);
        disp_command(DISP_CMD_COL_LSB + 4);
        for (col = 127; col >= 0; col--) {
            disp_data(0x00);
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
    for (int i = 7; i >= 0; i--) {
        if ((data >> i) & 1) {
            PORTB |= DISP_SI;
        } else {
            PORTB &= ~DISP_SI;
        }

        PORTB &= ~DISP_SCL;
        PORTB |= DISP_SCL;
    }
}