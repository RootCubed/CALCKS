#include "../common/buttons.h"
#include <stdio.h>

char buttonBuffer = -1;

void buttons_initialize() {}

void setButton(char btn) {
    buttonBuffer = btn;
}

int buttons_getPressed() {
    char prevBtn = buttonBuffer;
    buttonBuffer = -1;
    return (int) prevBtn;
}