#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "eep.h"

#define DISP_INIT_LEN 15

#define DISP_CS _BV(PORTB5)
#define DISP_RESET _BV(PORTB3)
#define DISP_A0 _BV(PORTB7)
#define DISP_SCL _BV(PORTB4)
#define DISP_SI _BV(PORTB6)

// commands
#define DISP_CMD_ONOFF 0xAE
#define DISP_CMD_START_LINE 0x40
#define DISP_CMD_PAGE 0xB0
#define DISP_CMD_COL_MSB 0x10
#define DISP_CMD_COL_LSB 0x00
#define DISP_CMD_ADC 0xA0
#define DISP_CMD_DISP_DIR 0xA6
#define DISP_CMD_ALL_ONOFF 0xA4
#define DISP_CMD_LCD_BIAS 0xA2
#define DISP_CMD_RESET 0xE2
#define DISP_CMD_COM_OUT_DIR 0xC0
#define DISP_CMD_POWER_CTRL 0x28
#define DISP_CMD_V0_RATIO 0x20
#define DISP_CMD_ELVOL_MODE 0x81
#define DISP_CMD_INDICATOR_ONOFF 0xAC
#define DISP_CMD_BOOSTER_RATIO_MODE 0xF8
#define DISP_CMD_NOP 0xE3

void disp_initialize();

void disp_clear();

void disp_command(char);
void disp_data(char);
void disp_update_data(char);
void disp_remove_data(char);

#define disp_setPage(page) disp_command(DISP_CMD_PAGE | page);
#define disp_setMSBCol(val) disp_command(DISP_CMD_COL_MSB | (val));
#define disp_setLSBCol(val) disp_command(DISP_CMD_COL_LSB | (val));
#define disp_setXPos(val) disp_setMSBCol((val + 4) >> 4 & 0xF); disp_setLSBCol((val + 4) & 0xF)

// In the 6 o'clock orientation, the display column address is offset by 4.
// Therefore, we have to subtract 4 from the xPos when drawing to the buffer.
#define realX (xPos - 4)

void disp_sendByte(char);

void disp_show();

#ifdef console
    void disp_print();
#endif

static const char disp_initSequence[DISP_INIT_LEN] = {
    DISP_CMD_START_LINE | 0b000000, // Start at line 0
    DISP_CMD_ADC | 0,               // ADC normal -> left to right
    DISP_CMD_COM_OUT_DIR | 0b1000,  // Select reverse common output
    DISP_CMD_DISP_DIR | 0,          // Set Display direction normal
    DISP_CMD_LCD_BIAS | 0,          // Set bias 1/9 - duty 1/65
    DISP_CMD_POWER_CTRL | 0b111,    // Power control: Booster on, Regulator on, Follower on
    DISP_CMD_BOOSTER_RATIO_MODE,    // Booster ratio:
    0x00,                           // 4x
    DISP_CMD_V0_RATIO | 7,          // V0 Voltage set: 7
    DISP_CMD_ELVOL_MODE,            // Electronic volume mode:
    0x16,                           // 22
    DISP_CMD_INDICATOR_ONOFF | 0,   // Static indicator off
    0x00,                           // Flashing mode: 0
    DISP_CMD_ALL_ONOFF | 0,         // Display all points: no
    DISP_CMD_ONOFF | 1              // Display on
};

#endif /* DISPLAY_H_ */