#include <stdio.h>
#include <string.h>

#include "../common/eep.h"

#define EEPROM_TOTAL_SIZE EEPROM_STARTUP_LEN + EEPROM_FONT_6x7_LEN + EEPROM_FONT_8x16_LEN

FILE *eepromdata;

char data[EEPROM_TOTAL_SIZE];

void eep_initialize() {
	eepromdata = fopen("eepromdata.bin", "rb");
    fread(data, 1, EEPROM_TOTAL_SIZE, eepromdata);
}

void eep_read_block(void *dst, int position, int length) {
    memcpy(dst, data + position, length);
}