#include <avr/io.h>
#include <avr/eeprom.h>

#include "../common/display.h"

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