#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "eep.h"

#define DISP_INIT_LEN 15

#define DISP_CS _BV(PORTB2)
#define DISP_RESET _BV(PORTB4)
#define DISP_A0 _BV(PORTB1)
#define DISP_SCL _BV(PORTB5)
#define DISP_SI _BV(PORTB3)

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

#define FNT_SM 0
#define FNT_MD 1

void disp_initialize();

void disp_clear();
void disp_drawChar(int, int, int, int);
void disp_drawImage();

void disp_command(char);
void disp_data(char);

void disp_sendByte(char);

#ifdef console
    void disp_print();
#endif

static const char disp_initSequence[DISP_INIT_LEN] = {
    DISP_CMD_START_LINE,            // Start at line 0
    DISP_CMD_ADC | 1,               // ADC reverse -> right to left
    DISP_CMD_COM_OUT_DIR,           // Select regular common output
    DISP_CMD_DISP_DIR,              // Set Display direction normal
    DISP_CMD_LCD_BIAS,              // Set bias 1/9 - duty 1/65
    DISP_CMD_POWER_CTRL | 0b111,    // Power control: Booster on, Regulator on, Follower on
    DISP_CMD_BOOSTER_RATIO_MODE,    // Booster ratio:
    0x00,                           // 4x
    DISP_CMD_V0_RATIO | 7,          // V0 Voltage set: 7
    DISP_CMD_ELVOL_MODE,            // Electronic volume mode:
    0x16,                           // 22
    DISP_CMD_INDICATOR_ONOFF,       // Static indicator off
    0x00,                           // Flashing mode: 0
    DISP_CMD_ALL_ONOFF,             // Display all points: no
    DISP_CMD_ONOFF | 1              // Display on
};

#endif /* DISPLAY_H_ */