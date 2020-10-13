#ifndef MATHINPUT_H_
#define MATHINPUT_H_

#include "term.h"
#include "gui.h"
#include "buttons.h"
#include <stdlib.h>

typedef struct {
    int font;
    int maxChars;
    int posX;
    int posY;
    char *buffer;
    int cursor;
} inputBox;

inputBox *mathinput_initBox(int, int, int, int);

void mathinput_freeBox(inputBox *);

void mathinput_buttonPress(inputBox *, int);

void mathinput_blinkCursor(inputBox *, int);

double mathinput_calcContent(inputBox *);

#endif