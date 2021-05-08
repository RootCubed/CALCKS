#include "../common/buttons.h"
#include <stdio.h>

const char BUTTON_MAP[35] = {
    off,        -1,      zero,         point,          enter,
	variable,   one,     two,          three,          plus,
	-1,         four,    five,         six,            minus,
	-1,         seven,   eight,        nine,           multiply,
	exponent,   -1,      bracket_open, bracket_close,  divide,
    -1,         btn_sin, btn_cos,      btn_tan,        del,
    f1,         f2,      f3,           f4,
	back
};

char buttonBuffer = -1;
int specialButtonBuffer = 0xFFFF;

void buttons_initialize() {}

void set_button(char btn) {
    buttonBuffer = btn;
}

void set_special_button(char btn) {
    specialButtonBuffer = 0xFFFF & ~btn;
}

char buttons_get_special() {
    char specialButtons = specialButtonBuffer;
    specialButtonBuffer = 0xFFFF;
    return specialButtons;
}

int buttons_getPressed() {
    char prevBtn = buttonBuffer;
    buttonBuffer = -1;
    return (int) prevBtn;
}