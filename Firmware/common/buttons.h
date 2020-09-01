#ifndef BUTTONS_H_
#define BUTTONS_H_

#define BUTTONS_SR_DATA _BV(PORTD7)
#define BUTTONS_SR_CLEAR _BV(PORTB2)
#define BUTTONS_SR_CLK _BV(PORTB1)
#define BUTTONS_ROW_0 _BV(PORTD2)
#define BUTTONS_ROW_1 _BV(PORTD3)
#define BUTTONS_ROW_2 _BV(PORTD4)
#define BUTTONS_ROW_3 _BV(PORTD5)
#define BUTTONS_ROW_4 _BV(PORTD6)

#define BUTTONS_NUM_COLUMNS 8

#define BUTTONS_SR_PULSE() PORTB &= ~BUTTONS_SR_CLK;PORTB |= BUTTONS_SR_CLK


extern const char BUTTON_MAP[30];


void buttons_initialize();
int buttons_getPressed();
#ifdef console
    void setButton(char);
#endif

#endif /* BUTTONS_H_ */