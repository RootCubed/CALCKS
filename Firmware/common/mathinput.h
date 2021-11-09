#ifndef MATHINPUT_H_
#define MATHINPUT_H_

#include "term.h"
#include "gui.h"
#include "buttons.h"
#include <stdlib.h>

#define CURSOR_ON 0
#define CURSOR_OFF 1
#define CURSOR_HIDDEN 2

typedef struct {
    int font;
    int maxChars;
    int posX;
    int posY;
    unsigned char *buffer;
    int cursor;
    int cursorX;
    int cursorY;
    int scroll;
    int length;
    int cursorBlinkState;
    int cursorFrameCount;
} inputBox;

inputBox *mathinput_initBox(int, int, int, int);

void mathinput_freeBox(inputBox *);

void mathinput_redraw(inputBox *);
void mathinput_drawChar(inputBox *, u8, int *, int *, int);

void mathinput_buttonPress(inputBox *, int);

void mathinput_cursorFrame(inputBox *);
void mathinput_setCursor(inputBox *, int);

void mathinput_clear(inputBox *);

int mathinput_checkSyntax(inputBox *);

double mathinput_calcContent(inputBox *);

#endif