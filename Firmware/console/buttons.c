#include "../common/buttons.h"
#include <stdio.h>

const char BUTTON_MAP[35] = {
    15, -1, 0, -1, 14,
	16, 1, 2, 3, 10,
	-1, 4, 5, 6, 11,
	-1, 7, 8, 9, 12,
	-1, -1, -1, -1, 13,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1
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