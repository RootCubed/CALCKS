#ifndef SOLVER_H_
#define SOLVER_H_

#include "gui.h"
#include "buttons.h"
#include "mathinput.h"
#include <stdio.h>

#define SOLV_LINEAR 0
#define SOLV_QUADRATIC 1

typedef struct {
    inputBox *a;
    inputBox *b;
    inputBox *c;
    int currentSelected;
    int solveType;
    double x1;
    double x2;
    int numSolutions;
} solver;

void solver_init();

void solver_free();

void solver_updateScreen();

void solver_redrawScreenLinear();
void solver_solveLinear();

void solver_redrawScreenQuadratic();
void solver_solveQuadratic();

void solver_buttonPress(int);

#endif