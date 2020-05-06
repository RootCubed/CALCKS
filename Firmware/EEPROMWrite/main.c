/*
 * EEPROMWrite.c
 *
 * Created: 19.03.2020 18:08:41
 * Author : liamb
 */ 

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "startup.h"
#include "font6x7.h"
#include "font8x16.h"
#include "../EEPROM/eep.h"

int main(void)
{
	DDRC = 0xFF;
	PORTC &= ~_BV(PORTC0);
	PORTC &= ~_BV(PORTC1);
	eep_initialize();
	eep_write_block(IMG_STARTUP, EEPROM_STARTUP, EEPROM_STARTUP_LEN);
    eep_write_block(*FONT_6x7, EEPROM_FONT_6x7, EEPROM_FONT_6x7_LEN);
    eep_write_block(*FONT_8x16, EEPROM_FONT_8x16, EEPROM_FONT_8x16_LEN);
	PORTC |= _BV(PORTC0);
}