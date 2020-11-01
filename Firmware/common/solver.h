#ifndef SOLVER_H_
#define SOLVER_H_

#include "gui.h"
#include "buttons.h"
#include "mathinput.h"
#include <stdio.h>

typedef struct {
    inputBox *m;
    inputBox *q;
    int currentSelected;
} solver;

void solver_init();

void solver_free();

void solver_updateScreen();

void solver_buttonPress(int);

#endif