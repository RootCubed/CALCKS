#include "../common/buttons.h"
#include <stdio.h>

const char BUTTON_MAP[30] = {
    14, // enter
    -1, // button doesn't work
    -1, 0, -1,
    10, // plus
    -1, // button doesn't work
    1, 2, 3,
    11, // minus
    -1, // button doesn't work
    4, 5, 6,
    12, // div
    -1, // button doesn't work
    7, 8, 9,
    13, // mult
    -1, -1, -1, -1
};

char buttonBuffer = -1;

void buttons_initialize() {}

void set_button(char btn) {
    buttonBuffer = btn;
}

int buttons_getPressed() {
    char prevBtn = buttonBuffer;
    buttonBuffer = -1;
    return (int) prevBtn;
}