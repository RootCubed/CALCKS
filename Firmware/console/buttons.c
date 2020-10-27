#include "../common/buttons.h"
#include <stdio.h>

const char BUTTON_MAP[35] = {
    off,        -1,    zero,         point,          enter,
	variable,   one,   two,          three,          plus,
	-1,         four,  five,         six,            minus,
	exponent,   seven, eight,        nine,           multiply,
	-1,         -1,    bracket_open, bracket_close,  divide,
    -1,         -1,    -1,           -1,             -1,
    f1,         f2,    f3,           f4,
	back
};

char buttonBuffer = -1;

void buttons_initialize() {}

void set_button(char btn) {
    buttonBuffer = btn;
}

char buttons_get_special() {
    return 0;
}

int buttons_getPressed() {
    char prevBtn = buttonBuffer;
    buttonBuffer = -1;
    return (int) prevBtn;
}