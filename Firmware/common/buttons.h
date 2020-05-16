/*
 * buttons.h
 *
 * Created: 20.03.2020 17:25:24
 *  Author: liamb
 */ 


#ifndef BUTTONS_H_
#define BUTTONS_H_

#define BUTTONS_SR_DATA _BV(PORTD7)
#define BUTTONS_SR_CLEAR _BV(PORTD6)
#define BUTTONS_SR_CLK _BV(PORTD5)
#define BUTTONS_ROW_0 _BV(PORTD4)
#define BUTTONS_ROW_1 _BV(PORTD3)
#define BUTTONS_FUNC_SWITCH _BV(PORTD2)

#define BUTTONS_NUM_COLUMNS 5

#define BUTTONS_SR_PULSE() PORTD &= ~BUTTONS_SR_CLK;PORTD |= BUTTONS_SR_CLK

void buttons_initialize();
int buttons_getPressed();
#ifdef console
    void setButton(char);
#endif

#endif /* BUTTONS_H_ */